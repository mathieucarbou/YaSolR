// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) Mathieu Carbou
 */
#include <yasolr.h>

#include <ringbuf.h>

#include <algorithm>
#include <cstdint>
#include <utility>

// Shelly LNM (Local Network Messaging) (see https://shelly-api-docs.shelly.cloud/gen2/General/LocalNetworkMessaging/)

// #define YASOLR_DEBUG_SHELLY_LNM 1

#if YASOLR_DEBUG_SHELLY_LNM
  #define DEBUG_LNM(...) ESP_LOGD(__VA_ARGS__)
#else
  #define DEBUG_LNM(...)
#endif

static AsyncUDP* shellyLnmUdp = nullptr;
baudvine::RingBuf<float, 15>* shellyLnmMessageRateBuffer = nullptr;
Mycila::Task* shellyLnmTask = nullptr;

static void onData(AsyncUDPPacket& packet) {
  // Shelly LNM wire protocol (see https://shelly-api-docs.shelly.cloud/gen2/DynamicComponents/LNM/#wire-protocol):
  //
  //   [header (12 bytes)] [payload (payload_len bytes)] [meta (meta_len bytes)]
  //
  // Binary header (little-endian, packed):
  //   offset 0  size 2  magic        0x53 0x4C ("SL")
  //   offset 2  size 1  version      protocol version (currently 0)
  //   offset 3  size 1  payload_type 1 = status/event
  //   offset 4  size 2  payload_len  payload length in bytes (LE)
  //   offset 6  size 2  meta_len     meta block length (currently 0)
  //   offset 8  size 4  reserved     reserved for future use
  //
  // For payload_type 1, the payload is a JSON object:
  //   {"device":"shellyproem50-08f9e0e5c2f8","ts":1787225613.78,"status":{"em1:0":{"id":0,"voltage":241.8,"current":3.559,"act_power":291.5,"aprt_power":862.4,"pf":0.36,"freq":50.0,"calibration":"factory"}}}
  //
  // The "status" object can have different keys (em1:0, em:0, switch:0, etc.) depending on the Shelly device.
  // We iterate over all status keys and extract the first one that contains voltage/current/power fields.

  const size_t len = packet.length();
  if (len < 12) {
    DEBUG_LNM(TAG, "[LNM] Packet too short for header: %u", len);
    return;
  }

  const uint8_t* data = packet.data();

  // Validate magic "SL" (0x53 0x4C)
  if (data[0] != 0x53 || data[1] != 0x4C) {
    DEBUG_LNM(TAG, "[LNM] Not a Shelly LNM message (bad magic)");
    return;
  }

  const uint8_t version = data[2];
  const uint8_t payloadType = data[3];
  const uint16_t payloadLen = data[4] | (data[5] << 8); // little-endian
  const uint16_t metaLen = data[6] | (data[7] << 8);    // little-endian

  // Only status/event payloads (type 1) contain the JSON we can parse
  if (payloadType != 1) {
    DEBUG_LNM(TAG, "[LNM] Unsupported payload_type: %u (version %u)", payloadType, version);
    return;
  }

  // Validate that the declared payload + meta fit in the received datagram
  if (12u + payloadLen + metaLen > len) {
    DEBUG_LNM(TAG, "[LNM] Truncated packet: header+payload+meta=%u > len=%u", 12u + payloadLen + metaLen, len);
    return;
  }

  if (payloadLen == 0) {
    DEBUG_LNM(TAG, "[LNM] Empty payload");
    return;
  }

  // Parse the JSON payload starting after the 12-byte header
  const char* payload = (const char*)data + 12;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, payloadLen);
  if (err) {
    ESP_LOGD(TAG, "[LNM] Failed to parse JSON: %s", err.c_str());
    return;
  }

  JsonObject root = doc.as<JsonObject>();
  JsonObject status = root["status"].as<JsonObject>();
  if (status.isNull()) {
    DEBUG_LNM(TAG, "[LNM] No status object found");
    return;
  }

  // The status object can have various keys (em1:0, em:0, etc.)
  // Iterate and find the first object that contains power/voltage measurements
  for (JsonPair kv : status) {
    JsonObject component = kv.value().as<JsonObject>();
    if (component.isNull())
      continue;

    // Check if this component has power-related fields
    if (!component["act_power"].is<float>() &&
        !component["voltage"].is<float>())
      continue;

#if YASOLR_DEBUG_SHELLY_LNM
    const size_t jsonSize = measureJson(component);
    String jsonString;
    jsonString.reserve(jsonSize + 1);
    serializeJson(component, jsonString);
    DEBUG_LNM(TAG, "[LNM] Component %s: %s", kv.key().c_str(), jsonString.c_str());
#endif

    Mycila::metric::Metrics metrics;
    metrics.voltage = component["voltage"] | NAN;
    metrics.current = component["current"] | NAN;
    metrics.power = component["act_power"] | NAN;
    metrics.apparentPower = component["aprt_power"] | NAN;
    metrics.powerFactor = component["pf"] | NAN;
    metrics.frequency = component["freq"] | NAN;

    grid.updateMetrics(std::move(metrics));
    pidTask.requestEarlyRun();

    // record stats
    if (shellyLnmMessageRateBuffer)
      shellyLnmMessageRateBuffer->push_back(millis() / 1000.0f);

    // Only process the first matching component
    break;
  }
}

void yasolr_configure_shelly_lnm() {
  if (grid.isUsing(Mycila::metric::Source::SHELLY_LNM)) {
    if (shellyLnmTask == nullptr) {
      ESP_LOGI(TAG, "Enable Shelly LNM");

      shellyLnmUdp = new AsyncUDP();
      shellyLnmUdp->onPacket(onData);

      if (shellyLnmMessageRateBuffer == nullptr)
        shellyLnmMessageRateBuffer = new baudvine::RingBuf<float, 15>();

      shellyLnmTask = new Mycila::Task("Shelly LNM", Mycila::Task::Type::ONCE, []() {
        shellyLnmUdp->close();
        const char* addr = config.getString(KEY_SHELLY_LNM_ADDR);
        const uint16_t port = config.get<uint16_t>(KEY_SHELLY_LNM_PORT);
        ESP_LOGI(TAG, "Enable Shelly LNM Listener on %s:%" PRIu16, addr, port);
        IPAddress mcastAddr;
        mcastAddr.fromString(addr);
        if (shellyLnmUdp->listenMulticast(mcastAddr, port)) {
          ESP_LOGI(TAG, "Shelly LNM multicast listener started on %s:%" PRIu16, addr, port);
        } else {
          ESP_LOGE(TAG, "Failed to start Shelly LNM multicast listener on %s:%" PRIu16, addr, port);
        }
      });

      unsafeTaskManager.addTask(*shellyLnmTask);
    }
  } else {
    if (shellyLnmTask != nullptr) {
      ESP_LOGI(TAG, "Disable Shelly LNM");

      unsafeTaskManager.removeTask(*shellyLnmTask);
      shellyLnmUdp->close();

      delete shellyLnmTask;
      delete shellyLnmUdp;
      delete shellyLnmMessageRateBuffer;

      shellyLnmTask = nullptr;
      shellyLnmUdp = nullptr;
      shellyLnmMessageRateBuffer = nullptr;
    }
  }
}

float yasolr_shelly_lnm_message_rate() {
  if (shellyLnmMessageRateBuffer && shellyLnmMessageRateBuffer->size() > 1) {
    float diff = shellyLnmMessageRateBuffer->back() - shellyLnmMessageRateBuffer->front();
    return diff == 0 ? 0 : shellyLnmMessageRateBuffer->size() / diff;
  }
  return 0;
}
