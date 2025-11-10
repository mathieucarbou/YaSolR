// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

AsyncUDP* udp = nullptr;
Mycila::CircularBuffer<float, 15>* udpMessageRateBuffer;
Mycila::Task* jsyRemoteTask = nullptr;

static void onData(AsyncUDPPacket packet) {
  // buffer[0] == MYCILA_UDP_MSG_TYPE_JSY_DATA (1)
  // buffer[1] == size_t (4)
  // buffer[5] == MsgPack (?)
  // buffer[5 + size] == CRC32 (4)

  size_t len = packet.length();
  uint8_t* buffer = packet.data();

  if (len < 5 || buffer[0] != YASOLR_UDP_MSG_TYPE_JSY_DATA)
    return;

  size_t size;
  memcpy(&size, buffer + 1, 4);

  if (len != size + 9)
    return;

  // crc32
  FastCRC32 crc32;
  crc32.add(buffer, size + 5);
  uint32_t crc = crc32.calc();

  if (memcmp(&crc, buffer + size + 5, 4) != 0)
    return;

  udpMessageRateBuffer->add(millis() / 1000.0f);

  JsonDocument doc;
  deserializeMsgPack(doc, buffer + 5, size);
  // serializeJsonPretty(doc, Serial);

  switch (doc["model"].as<uint16_t>()) {
    case MYCILA_JSY_MK_1031:
      // JSY1030 has no sign: it cannot be used to measure the grid
      break;

    case MYCILA_JSY_MK_163:
    case MYCILA_JSY_MK_227:
    case MYCILA_JSY_MK_229: {
      grid.metrics(Mycila::Grid::Source::JSY_REMOTE).update({
        .apparentPower = doc["apparent_power"] | NAN,
        .current = doc["current"] | NAN,
        .energy = doc["active_energy_imported"] | static_cast<uint32_t>(0),
        .energyReturned = doc["active_energy_returned"] | static_cast<uint32_t>(0),
        .frequency = doc["frequency"] | NAN,
        .power = doc["active_power"] | NAN,
        .powerFactor = doc["power_factor"] | NAN,
        .voltage = doc["voltage"] | NAN,
      });
      break;
    }
    case MYCILA_JSY_MK_193:
    case MYCILA_JSY_MK_194: {
      grid.metrics(Mycila::Grid::Source::JSY_REMOTE).update({
        .apparentPower = doc["channel2"]["apparent_power"] | NAN,
        .current = doc["channel2"]["current"] | NAN,
        .energy = doc["channel2"]["active_energy_imported"] | static_cast<uint32_t>(0),
        .energyReturned = doc["channel2"]["active_energy_returned"] | static_cast<uint32_t>(0),
        .frequency = doc["channel2"]["frequency"] | NAN,
        .power = doc["channel2"]["active_power"] | NAN,
        .powerFactor = doc["channel2"]["power_factor"] | NAN,
        .voltage = doc["channel2"]["voltage"] | NAN,
      });
      router.metrics().update({
        .source = Mycila::Router::Source::JSY_REMOTE,
        .apparentPower = doc["channel1"]["apparent_power"] | 0.0f,
        .current = doc["channel1"]["current"] | 0.0f,
        .energy = (doc["channel1"]["active_energy"] | static_cast<uint32_t>(0)) + (doc["channel1"]["active_energy_returned"] | static_cast<uint32_t>(0)), // if the clamp is installed reversed
        .power = std::abs(doc["channel1"]["active_power"] | 0.0f),                                                                                         // if the clamp is installed reversed
        .powerFactor = doc["channel1"]["power_factor"] | NAN,
        .resistance = doc["channel1"]["resistance"] | NAN,
        .thdi = doc["channel1"]["thdi_0"] | NAN,
      });
      break;
    }
    case MYCILA_JSY_MK_333: {
      JsonObject aggregate = doc["aggregate"].as<JsonObject>();
      grid.metrics(Mycila::Grid::Source::JSY_REMOTE).update({
        .apparentPower = aggregate["apparent_power"] | NAN,
        .current = aggregate["current"] | NAN,
        .energy = aggregate["active_energy_imported"] | static_cast<uint32_t>(0),
        .energyReturned = aggregate["active_energy_returned"] | static_cast<uint32_t>(0),
        .frequency = aggregate["frequency"] | NAN,
        .power = aggregate["active_power"] | NAN,
        .powerFactor = aggregate["power_factor"] | NAN,
        .voltage = aggregate["voltage"] | NAN,
      });
      break;
    }
    default:
      break;
  }

  if (grid.isUsing(Mycila::Grid::Source::JSY_REMOTE)) {
    yasolr_divert();
  }
}

void yasolr_configure_jsy_remote() {
  if (config.getBool(KEY_ENABLE_JSY_REMOTE)) {
    if (jsyRemoteTask == nullptr) {
      ESP_LOGI(TAG, "Enable Remote JSY");

      udp = new AsyncUDP();
      udp->onPacket(onData);

      grid.metrics(Mycila::Grid::Source::JSY_REMOTE).setExpiration(10000);

      udpMessageRateBuffer = new Mycila::CircularBuffer<float, 15>();

      jsyRemoteTask = new Mycila::Task("Remote JSY", Mycila::Task::Type::ONCE, [](void* params) {
        const uint16_t udpPort = config.getLong(KEY_UDP_PORT);
        ESP_LOGI(TAG, "Enable Remote JSY Listener on port %" PRIu16, udpPort);
        udp->listen(udpPort);
      });

      coreTaskManager.addTask(*jsyRemoteTask);

      Mycila::TaskMonitor.addTask("async_udp"); // AsyncUDP (stack size cannot be set)
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
    }
  }
}
