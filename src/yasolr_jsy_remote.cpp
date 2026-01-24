// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

#include <algorithm>
#include <utility>

#define YASOLR_DEBUG_UDP 0

#if YASOLR_DEBUG_UDP
  #define DEBUG_UDP(...) ESP_LOGD(__VA_ARGS__)
#else
  #define DEBUG_UDP(...)
#endif

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

static void processJSON(JsonDocument& doc) {
  Mycila::Grid::Metrics metrics;
  metrics.source = Mycila::Grid::Source::JSY_REMOTE;
  JsonObject root = doc.as<JsonObject>();

#if YASOLR_DEBUG_UDP
  const size_t jsonSize = measureJson(doc);
  String jsonString;
  jsonString.reserve(jsonSize + 1);
  serializeJson(root, jsonString);
  DEBUG_UDP(TAG, "[UDP] JSY Data: %s", jsonString.c_str());
  DEBUG_UDP(TAG, "[UDP] JSY Model: %" PRIu16, root["model"].as<uint16_t>());
#endif

  switch (root["model"].as<uint16_t>()) {
    case MYCILA_JSY_MK_163:
    case MYCILA_JSY_MK_227:
    case MYCILA_JSY_MK_229: {
      metrics.apparentPower = root["apparent_power"] | NAN;
      metrics.current = root["current"] | NAN;
      metrics.energy = root["active_energy_imported"] | static_cast<uint32_t>(0);
      metrics.energyReturned = root["active_energy_returned"] | static_cast<uint32_t>(0);
      metrics.frequency = root["frequency"] | NAN;
      metrics.power = root["active_power"] | NAN;
      metrics.powerFactor = root["power_factor"] | NAN;
      metrics.voltage = root["voltage"] | NAN;
      break;
    }
    case MYCILA_JSY_MK_193:
    case MYCILA_JSY_MK_194: {
      if (root["channel1"].is<JsonObject>()) {
        Mycila::Router::Metrics routerMetrics;
        routerMetrics.source = Mycila::Router::Source::JSY_REMOTE;
        routerMetrics.apparentPower = root["channel1"]["apparent_power"] | NAN;
        routerMetrics.current = root["channel1"]["current"] | NAN;
        routerMetrics.energy = (root["channel1"]["active_energy"] | static_cast<uint32_t>(0)) + (root["channel1"]["active_energy_returned"] | static_cast<uint32_t>(0)); // if the clamp is installed reversed
        routerMetrics.power = std::abs(root["channel1"]["active_power"] | NAN);                                                                                          // if the clamp is installed reversed
        routerMetrics.powerFactor = root["channel1"]["power_factor"] | NAN;
        routerMetrics.resistance = root["channel1"]["resistance"] | NAN;
        routerMetrics.thdi = root["channel1"]["thdi_0"] | NAN;
        routerMetrics.voltage = root["channel1"]["dimmed_voltage"] | NAN;
        router.updateMetrics(std::move(routerMetrics));
      }
      if (root["channel2"].is<JsonObject>()) {
        metrics.apparentPower = root["channel2"]["apparent_power"] | NAN;
        metrics.current = root["channel2"]["current"] | NAN;
        metrics.energy = root["channel2"]["active_energy_imported"] | static_cast<uint32_t>(0);
        metrics.energyReturned = root["channel2"]["active_energy_returned"] | static_cast<uint32_t>(0);
        metrics.frequency = root["channel2"]["frequency"] | NAN;
        metrics.power = root["channel2"]["active_power"] | NAN;
        metrics.powerFactor = root["channel2"]["power_factor"] | NAN;
        metrics.voltage = root["channel2"]["voltage"] | NAN;
      }
      break;
    }
    case MYCILA_JSY_MK_333: {
      metrics.apparentPower = root["aggregate"]["apparent_power"] | NAN;
      metrics.current = root["aggregate"]["current"] | NAN;
      metrics.energy = root["aggregate"]["active_energy_imported"] | static_cast<uint32_t>(0);
      metrics.energyReturned = root["aggregate"]["active_energy_returned"] | static_cast<uint32_t>(0);
      metrics.frequency = root["aggregate"]["frequency"] | NAN;
      metrics.power = root["aggregate"]["active_power"] | NAN;
      metrics.powerFactor = root["aggregate"]["power_factor"] | NAN;
      metrics.voltage = root["aggregate"]["voltage"] | NAN;
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
  // buffer[0] == YASOLR_UDP_MSG_TYPE_JSY_DATA (1)
  // buffer[1] == message ID (4) - uint32_t
  // buffer[5] == jsonSize (4) - size_t
  // buffer[9] == MsgPack (?)
  // buffer[9 + size] == CRC32 (4)

  const size_t len = packet.length();
  const uint8_t* buffer = packet.data();

  DEBUG_UDP(TAG, "[UDP] Received packet of size %" PRIu32, len);

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
    } else {
      DEBUG_UDP(TAG, "[UDP] New MycilaJSYApp packet detected!");
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
    size_t payloadSize = 0;
    memcpy(&payloadSize, buffer + 5, 4);

    // validate message size
    if (!payloadSize) {
      ESP_LOGD(TAG, "[UDP] Invalid message size: 0");
      return;
    } else {
      DEBUG_UDP(TAG, "[UDP] Internal payload size: %" PRIu32, payloadSize);
    }

    // arbitrary limit to avoid memory exhaustion
    if (payloadSize > 4096) {
      ESP_LOGD(TAG, "[UDP] Message size too large: %" PRIu32, payloadSize);
      return;
    }

    // compute total reassembled message size and allocate buffer
    reassembledMessage = new UDPMessage(payloadSize);
    DEBUG_UDP(TAG, "[UDP] Allocated new message buffer of size %" PRIu32, reassembledMessage->remaining);

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
    DEBUG_UDP(TAG, "[UDP] Validating next packet");

    // check that the packet comes from the same sender
    uint8_t mac[6];
    packet.remoteMac(mac);
    if (memcmp(mac, reassembledMessage->sourceMAC, 6) != 0) {
      ESP_LOGD(TAG, "[UDP] Discarding packet from different sender. Expected: %02X:%02X:%02X:%02X:%02X:%02X, got %02X:%02X:%02X:%02X:%02X:%02X", reassembledMessage->sourceMAC[0], reassembledMessage->sourceMAC[1], reassembledMessage->sourceMAC[2], reassembledMessage->sourceMAC[3], reassembledMessage->sourceMAC[4], reassembledMessage->sourceMAC[5], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      return;
    } else {
      DEBUG_UDP(TAG, "[UDP] Packet sender validated: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    // check if the packet comes by the same interface
    if (packet.localIP() != reassembledMessage->interface && packet.localIPv6() != reassembledMessage->interface) {
      ESP_LOGD(TAG, "[UDP] Discarding packet from different interface. Expected: %s, got IPv4: %s, IPv6: %s", reassembledMessage->interface.toString().c_str(), packet.localIP().toString().c_str(), packet.localIPv6().toString().c_str());
      return;
    } else {
      DEBUG_UDP(TAG, "[UDP] Packet interface validated");
    }

    // additional packet validated
    reassembledMessage->lastPacketTime = millis();
  }

  // assemble packets
  reassembledMessage->append(buffer, len);

  if (reassembledMessage->remaining) {
    DEBUG_UDP(TAG, "[UDP] Waiting for more packets to complete message. Remaining size: %" PRIu32, reassembledMessage->remaining);
    return;
  } else {
    DEBUG_UDP(TAG, "[UDP] Reassembled complete message of size %" PRIu32, reassembledMessage->index);
  }

  // we have finished reassembling packets
  // verify CRC32
  if (!reassembledMessage->crcValid()) {
    ESP_LOGD(TAG, "[UDP] CRC32 mismatch");
    delete reassembledMessage;
    reassembledMessage = nullptr;
    return;
  } else {
    DEBUG_UDP(TAG, "[UDP] CRC32 valid");
  }

  // extract message
  JsonDocument doc;
  DeserializationError err = reassembledMessage->parseMsgPack(doc);

  if (err) {
    ESP_LOGD(TAG, "[UDP] Failed to parse MsgPack: %s", err.c_str());
    delete reassembledMessage;
    reassembledMessage = nullptr;
    return;
  } else {
    DEBUG_UDP(TAG, "[UDP] MsgPack parsed successfully");
  }

  processJSON(doc);

  delete reassembledMessage;
  reassembledMessage = nullptr;
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
