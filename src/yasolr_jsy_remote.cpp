// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

#include <algorithm>
#include <utility>

AsyncUDP* udp = nullptr;
Mycila::CircularBuffer<float, 15>* udpMessageRateBuffer = nullptr;
Mycila::Task* jsyRemoteTask = nullptr;

static uint8_t* reassembledMessage = nullptr;
static size_t reassembledMessageIndex = 0;
static size_t reassembledMessageRemaining = 0;
static uint8_t reassembledMessageMAC[6] = {0};
static uint32_t lastPacketTime = 0;

static void processJSON(const JsonDocument& doc) {
  Mycila::Grid::Metrics metrics;
  metrics.source = Mycila::Grid::Source::JSY_REMOTE;

  switch (doc["model"].as<uint16_t>()) {
    case MYCILA_JSY_MK_163:
    case MYCILA_JSY_MK_227:
    case MYCILA_JSY_MK_229: {
      metrics.apparentPower = doc["apparent_power"] | NAN;
      metrics.current = doc["current"] | NAN;
      metrics.energy = doc["active_energy_imported"] | static_cast<uint32_t>(0);
      metrics.energyReturned = doc["active_energy_returned"] | static_cast<uint32_t>(0);
      metrics.frequency = doc["frequency"] | NAN;
      metrics.power = doc["active_power"] | NAN;
      metrics.powerFactor = doc["power_factor"] | NAN;
      metrics.voltage = doc["voltage"] | NAN;
      break;
    }
    case MYCILA_JSY_MK_193:
    case MYCILA_JSY_MK_194: {
      if (doc["channel1"].is<JsonObject>()) {
        Mycila::Router::Metrics routerMetrics;
        routerMetrics.source = Mycila::Router::Source::JSY_REMOTE;
        routerMetrics.apparentPower = doc["channel1"]["apparent_power"] | NAN;
        routerMetrics.current = doc["channel1"]["current"] | NAN;
        routerMetrics.energy = (doc["channel1"]["active_energy"] | static_cast<uint32_t>(0)) + (doc["channel1"]["active_energy_returned"] | static_cast<uint32_t>(0)); // if the clamp is installed reversed
        routerMetrics.power = std::abs(doc["channel1"]["active_power"] | NAN);                                                                                         // if the clamp is installed reversed
        routerMetrics.powerFactor = doc["channel1"]["power_factor"] | NAN;
        routerMetrics.resistance = doc["channel1"]["resistance"] | NAN;
        routerMetrics.thdi = doc["channel1"]["thdi_0"] | NAN;
        routerMetrics.voltage = doc["channel1"]["dimmed_voltage"] | NAN;
        router.updateMetrics(std::move(routerMetrics));
      }
      if (doc["channel2"].is<JsonObject>()) {
        metrics.apparentPower = doc["channel2"]["apparent_power"] | NAN;
        metrics.current = doc["channel2"]["current"] | NAN;
        metrics.energy = doc["channel2"]["active_energy_imported"] | static_cast<uint32_t>(0);
        metrics.energyReturned = doc["channel2"]["active_energy_returned"] | static_cast<uint32_t>(0);
        metrics.frequency = doc["channel2"]["frequency"] | NAN;
        metrics.power = doc["channel2"]["active_power"] | NAN;
        metrics.powerFactor = doc["channel2"]["power_factor"] | NAN;
        metrics.voltage = doc["channel2"]["voltage"] | NAN;
      }
      break;
    }
    case MYCILA_JSY_MK_333: {
      metrics.apparentPower = doc["aggregate"]["apparent_power"] | NAN;
      metrics.current = doc["aggregate"]["current"] | NAN;
      metrics.energy = doc["aggregate"]["active_energy_imported"] | static_cast<uint32_t>(0);
      metrics.energyReturned = doc["aggregate"]["active_energy_returned"] | static_cast<uint32_t>(0);
      metrics.frequency = doc["aggregate"]["frequency"] | NAN;
      metrics.power = doc["aggregate"]["active_power"] | NAN;
      metrics.powerFactor = doc["aggregate"]["power_factor"] | NAN;
      metrics.voltage = doc["aggregate"]["voltage"] | NAN;
      break;
    }
    default:
      // unknown model => do not divert
      return;
  }

  grid.updateMetrics(std::move(metrics));

  if (grid.isUsing(Mycila::Grid::Source::JSY_REMOTE)) {
    pidTask.requestEarlyRun();
  }

  udpMessageRateBuffer->add(millis() / 1000.0f);
}

static void onData(AsyncUDPPacket& packet) {
  // buffer[0] == MYCILA_UDP_MSG_TYPE_JSY_DATA (1)
  // buffer[1] == size_t (4)
  // buffer[5] == MsgPack (?)
  // buffer[5 + size] == CRC32 (4)

  const size_t len = packet.length();
  const uint8_t* buffer = packet.data();

  // ESP_LOGD(TAG, "[UDP] Received packet of size %" PRIu32, len);

  // this is a new message ?
  if (reassembledMessage == nullptr) {
    // check message validity
    if (len < 10 || buffer[0] != YASOLR_UDP_MSG_TYPE_JSY_DATA)
      return;

    // extract message size
    memcpy(&reassembledMessageRemaining, buffer + 1, 4);

    // a message must have a length
    if (!reassembledMessageRemaining) {
      ESP_LOGD(TAG, "[UDP] Invalid message size: 0");
      return;
    }

    if (reassembledMessageRemaining > 4096) { // arbitrary limit to avoid memory exhaustion
      ESP_LOGD(TAG, "[UDP] Message size too large: %" PRIu32, reassembledMessageRemaining);
      reassembledMessageRemaining = 0;
      return;
    }

    reassembledMessageRemaining += 9; // add header and CRC32 size

    // ESP_LOGD(TAG, "[UDP] Allocating new message of size %" PRIu32, reassembledMessageRemaining);

    // allocate new message buffer
    reassembledMessageIndex = 0;
    reassembledMessage = new uint8_t[reassembledMessageRemaining];

    // save sender MAC address
    packet.remoteMac(reassembledMessageMAC);

    // track the time of last packet
    lastPacketTime = millis();
  }

  // assemble packets
  if (reassembledMessage != nullptr) {
    // check that the packet comes from the same sender
    uint8_t mac[6];
    packet.remoteMac(mac);
    if (memcmp(mac, reassembledMessageMAC, 6) != 0) {
      ESP_LOGD(TAG, "[UDP] Discarding packet from different sender. Expected: %02X:%02X:%02X:%02X:%02X:%02X, got %02X:%02X:%02X:%02X:%02X:%02X", reassembledMessageMAC[0], reassembledMessageMAC[1], reassembledMessageMAC[2], reassembledMessageMAC[3], reassembledMessageMAC[4], reassembledMessageMAC[5], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      return;
    }

    size_t n = std::min(reassembledMessageRemaining, len);
    // ESP_LOGD(TAG, "[UDP] Appending packet of size %" PRIu32, n);
    memcpy(reassembledMessage + reassembledMessageIndex, buffer, n);
    reassembledMessageIndex += n;
    reassembledMessageRemaining -= n;

    // track the time of last packet
    lastPacketTime = millis();
  }

  // we are waiting for more packets ?
  if (reassembledMessageRemaining) {
    // check for timeout (10 seconds) in case a sender disappears mid-message
    if (millis() - lastPacketTime > 10000) {
      ESP_LOGD(TAG, "[UDP] Timeout waiting for more packets from sender: %02X:%02X:%02X:%02X:%02X:%02X", reassembledMessageMAC[0], reassembledMessageMAC[1], reassembledMessageMAC[2], reassembledMessageMAC[3], reassembledMessageMAC[4], reassembledMessageMAC[5]);
      delete[] reassembledMessage;
      reassembledMessage = nullptr;
      reassembledMessageIndex = 0;
      reassembledMessageRemaining = 0;
    }
    return;
  }

  // we have finished reassembling packets
  // ESP_LOGD(TAG, "[UDP] Reassembled full message of size %" PRIu32, reassembledMessageIndex);

  // CRC32 check (last 4 bytes are the CRC32)
  FastCRC32 crc32;
  crc32.add(reassembledMessage, reassembledMessageIndex - 4);
  uint32_t crc = crc32.calc();

  // verify CRC32
  if (memcmp(&crc, reassembledMessage + reassembledMessageIndex - 4, 4) != 0) {
    ESP_LOGD(TAG, "[UDP] CRC32 mismatch - expected 0x%08" PRIX32 ", got 0x%08" PRIX32, crc, *((uint32_t*)(reassembledMessage + reassembledMessageIndex - 4))); // NOLINT
    delete[] reassembledMessage;
    reassembledMessage = nullptr;
    reassembledMessageIndex = 0;
    reassembledMessageRemaining = 0;
    return;
  }

  // extract message
  // ESP_LOGD(TAG, "[UDP] CRC32 valid - parsing message");
  JsonDocument doc;
  DeserializationError err = deserializeMsgPack(doc, reassembledMessage + 5, reassembledMessageIndex - 9);

  // cleanup reassembled message buffer
  delete[] reassembledMessage;
  reassembledMessage = nullptr;
  reassembledMessageIndex = 0;
  reassembledMessageRemaining = 0;

  if (err) {
    ESP_LOGD(TAG, "[UDP] Failed to parse MsgPack: %s", err.c_str());
    return;
  }

  processJSON(doc);
}

void yasolr_configure_jsy_remote() {
  if (config.get<bool>(KEY_ENABLE_JSY_REMOTE)) {
    if (jsyRemoteTask == nullptr) {
      ESP_LOGI(TAG, "Enable Remote JSY");

      udp = new AsyncUDP();
      udp->onPacket(onData);

      udpMessageRateBuffer = new Mycila::CircularBuffer<float, 15>();

      jsyRemoteTask = new Mycila::Task("Remote JSY", Mycila::Task::Type::ONCE, []() {
        const uint16_t udpPort = config.get<uint16_t>(KEY_UDP_PORT);
        ESP_LOGI(TAG, "Enable Remote JSY Listener on port %" PRIu16, udpPort);
        udp->listen(udpPort);
      });

      coreTaskManager.addTask(*jsyRemoteTask);
    }
  } else {
    if (jsyRemoteTask != nullptr) {
      ESP_LOGI(TAG, "Disable Remote JSY");

      Mycila::TaskMonitor.removeTask("async_udp");

      coreTaskManager.removeTask(*jsyRemoteTask);
      udp->close();

      delete jsyRemoteTask;
      delete udp;
      delete udpMessageRateBuffer;

      jsyRemoteTask = nullptr;
      udp = nullptr;
      udpMessageRateBuffer = nullptr;

      grid.deleteMetrics(Mycila::Grid::Source::JSY_REMOTE);
      router.deleteMetrics(Mycila::Router::Source::JSY_REMOTE);
    }
  }
}
