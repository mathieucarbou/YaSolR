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

class UDPMessage {
  public:
    explicit UDPMessage(size_t dataSize) : data(new uint8_t[dataSize + 13]), remaining(dataSize + 13), index(0), lastPacketTime(millis()) {}

    ~UDPMessage() {
      delete[] data;
    }

    void append(const uint8_t* buffer, size_t len) {
      const size_t n = std::min(remaining, len);
      // ESP_LOGD(TAG, "[UDP] Appending packet of size %" PRIu32 " to message buffer at position %" PRIu32, n, index);
      memcpy(data + index, buffer, n);
      index += n;
      remaining -= n;
    }

    bool crcValid() {
      FastCRC32 crc32;
      crc32.add(data, index - 4);
      uint32_t crc = crc32.calc();
      return memcmp(&crc, data + index - 4, 4) == 0;
    }

    DeserializationError parseMsgPack(JsonDocument& doc) {
      return deserializeMsgPack(doc, data + 9, index - 13);
    }

    uint8_t* data = nullptr;
    size_t remaining = 0;
    size_t index = 0;
    uint32_t lastPacketTime = 0;
    uint8_t sourceMAC[6] = {0};
    IPAddress interface;
};

static uint32_t lastMessageID = 0;
static UDPMessage* reassembledMessage = nullptr;

static void processJSON(const JsonDocument& doc) {
  Mycila::Grid::Metrics metrics;
  metrics.source = Mycila::Grid::Source::JSY_REMOTE;

  // ESP_LOGD(TAG, "[UDP] JSY Model: %" PRIu16, doc["model"].as<uint16_t>());

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
  // buffer[1] == message ID (4) - uint32_t
  // buffer[5] == jsonSize (4) - size_t
  // buffer[9] == MsgPack (?)
  // buffer[9 + size] == CRC32 (4)

  const size_t len = packet.length();
  const uint8_t* buffer = packet.data();

  // ESP_LOGD(TAG, "[UDP] Received packet of size %" PRIu32, len);

  // if we are waiting for more packets, check for timeout (10 seconds) in case a sender disappears mid-message
  // we free the buffer and reset state before processing the new packet
  if (reassembledMessage && reassembledMessage->remaining && millis() - reassembledMessage->lastPacketTime > 10000) {
    ESP_LOGD(TAG, "[UDP] Timeout waiting for more packets from sender: %02X:%02X:%02X:%02X:%02X:%02X", reassembledMessage->sourceMAC[0], reassembledMessage->sourceMAC[1], reassembledMessage->sourceMAC[2], reassembledMessage->sourceMAC[3], reassembledMessage->sourceMAC[4], reassembledMessage->sourceMAC[5]);
    delete reassembledMessage;
    reassembledMessage = nullptr;
  }

  // this is a new message ?
  if (reassembledMessage == nullptr) {
    // check message validity
    if (len <= 13 || buffer[0] != YASOLR_UDP_MSG_TYPE_JSY_DATA) {
      ESP_LOGD(TAG, "[UDP] Invalid packet received of size %" PRIu32 ", not coming from a supported Mycila JSY App version!", len);
      return;
    }

    // extract message ID
    uint32_t messageID = 0;
    memcpy(&messageID, buffer + 1, 4);

    // check for duplicate message ID
    if (messageID == lastMessageID) {
      ESP_LOGD(TAG, "[UDP] Received duplicate message ID: %" PRIu32 ". Please remove WiFi SSID or disconnect ETH!", messageID);
      return;
    }

    // extract message size
    size_t jsonSize = 0;
    memcpy(&jsonSize, buffer + 5, 4);

    // validate message size
    if (!jsonSize) {
      ESP_LOGD(TAG, "[UDP] Invalid message size: 0");
      return;
    }

    // arbitrary limit to avoid memory exhaustion
    if (jsonSize > 4096) {
      ESP_LOGD(TAG, "[UDP] Message size too large: %" PRIu32, jsonSize);
      return;
    }

    // compute total reassembled message size and allocate buffer
    // ESP_LOGD(TAG, "[UDP] Allocating new message buffer of size %" PRIu32, jsonSize + 13);
    reassembledMessage = new (std::nothrow) UDPMessage(jsonSize);

    if (reassembledMessage == nullptr) {
      ESP_LOGD(TAG, "[UDP] Failed to allocate memory for data size %" PRIu32, jsonSize);
      return;
    }

    // save last message ID
    lastMessageID = messageID;

    // save sender MAC address
    packet.remoteMac(reassembledMessage->sourceMAC);

    // save interface IP address
    reassembledMessage->interface = packet.localIP();
    if (reassembledMessage->interface == IPAddress()) {
      reassembledMessage->interface = packet.localIPv6();
    }

  } else {
    // ESP_LOGD(TAG, "[UDP] Validating next packet");

    // check that the packet comes from the same sender
    uint8_t mac[6];
    packet.remoteMac(mac);
    if (memcmp(mac, reassembledMessage->sourceMAC, 6) != 0) {
      ESP_LOGD(TAG, "[UDP] Discarding packet from different sender. Expected: %02X:%02X:%02X:%02X:%02X:%02X, got %02X:%02X:%02X:%02X:%02X:%02X", reassembledMessage->sourceMAC[0], reassembledMessage->sourceMAC[1], reassembledMessage->sourceMAC[2], reassembledMessage->sourceMAC[3], reassembledMessage->sourceMAC[4], reassembledMessage->sourceMAC[5], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      return;
    }

    // check if the packet comes by the same interface
    if (packet.localIP() != reassembledMessage->interface && packet.localIPv6() != reassembledMessage->interface) {
      ESP_LOGD(TAG, "[UDP] Discarding packet from different interface. Expected: %s, got IPv4: %s, IPv6: %s", reassembledMessage->interface.toString().c_str(), packet.localIP().toString().c_str(), packet.localIPv6().toString().c_str());
      return;
    }

    // additional packet validated
    reassembledMessage->lastPacketTime = millis();
  }

  // assemble packets
  reassembledMessage->append(buffer, len);

  if (reassembledMessage->remaining) {
    // ESP_LOGD(TAG, "[UDP] Waiting for more packets to complete message. Remaining size: %" PRIu32, reassembledMessage->remaining);
    return;
  }

  // we have finished reassembling packets
  // verify CRC32
  if (!reassembledMessage->crcValid()) {
    ESP_LOGD(TAG, "[UDP] CRC32 mismatch");
    delete reassembledMessage;
    reassembledMessage = nullptr;
    return;
  }

  // extract message
  JsonDocument doc;
  DeserializationError err = reassembledMessage->parseMsgPack(doc);

  // cleanup reassembled message buffer
  delete reassembledMessage;
  reassembledMessage = nullptr;

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
