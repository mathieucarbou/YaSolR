// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaVictron.h>

#include <string>

#ifdef MYCILA_LOGGER_SUPPORT
  #include <MycilaLogger.h>
extern Mycila::Logger logger;
  #define LOGD(tag, format, ...) logger.debug(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) logger.info(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) logger.warn(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) logger.error(tag, format, ##__VA_ARGS__)
#else
  #define LOGD(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#endif

#define TAG "VICTRON"

// Define a function to parse a signed 16-bit integer from a Modbus message response
// (Modbus uses big-endian byte order)
// The response is a Modbus message, which is a vector of bytes
// The function returns a signed 16-bit integer
// scale : 10.0f for x10scale, 1.0f for no scale, 0.1f for 1 decimal, 0.01f for 2 decimals
// signedValue : true if the value is signed, false if the value is unsigned
static int16_t parseSignedorUnsignedInt16(ModbusMessage response, int offset, float scale, bool signedValue) {
  int32_t value = 0;
  // Combine the two bytes of the response into a signed 16-bit integer
  // (Modbus uses big-endian byte order, so we have to swap the bytes)
  //  - response[3] is the high byte
  //  - response[4] is the low byte
  *((unsigned char*)&value + 1) = response[offset];
  *((unsigned char*)&value + 0) = response[offset + 1];

  // Convert the 16-bit integer to a signed value
  // (Modbus uses two's complement for negative values)
  if (signedValue) {
    if (value >= 32768) {
      value = value - 65536;
    }
  }

  // Apply the scaling factor
  value = value * scale;
  return value;
}

void Mycila::Victron::begin(const char* host, uint16_t port) {
  if (_client) {
    return;
  }

  LOGI(TAG, "Connecting to Victron Modbus TCP Server %s:%" PRIu16 "", host, port);
  _client = new ModbusClientTCPasync(IPAddress(host), port);
  _client->setTimeout(DEFAULTTIMEOUT);
  _client->setIdleTimeout(DEFAULTIDLETIME);

  _client->onDataHandler([this](ModbusMessage response, uint32_t token) {
    _lastError = "";

    // ======================== AC Input based on CCGX-Modbus-TCP-register-list-3.40 ==========================
    // | SLAVE_ID | Description           | Register | Type   | Offset | Scale | Range               | Unit   |
    // |----------|-----------------------|----------|--------|--------|-------|---------------------|--------|
    // | 228      | Input voltage phase 1 | 3        | uint16 | 3 (*)  | 0.1f  | 0 to 6553.5         | V      |
    // | 228      | Input voltage phase 2 | 4        | uint16 | 5      | 0.1f  | 0 to 6553.5         | V      |
    // | 228      | Input voltage phase 3 | 5        | uint16 | 7      | 0.1f  | 0 to 6553.5         | V      |
    // | 228      | Input current phase 1 | 6        | int16  | 9 (*)  | 0.1f  | -3276.8 to 3276.7   | A AC   |
    // | 228      | Input current phase 2 | 7        | int16  | 11     | 0.1f  | -3276.8 to 3276.7   | A AC   |
    // | 228      | Input current phase 3 | 8        | int16  | 13     | 0.1f  | -3276.8 to 3276.7   | A AC   |
    // | 228      | Input frequency 1     | 9        | int16  | 15 (*) | 0.01f | -327.68 to 327.67   | Hz     |
    // | 228      | Input frequency 2     | 10       | int16  | 17     | 0.01f | -327.68 to 327.67   | Hz     |
    // | 228      | Input frequency 3     | 11       | int16  | 19     | 0.01f | -327.68 to 327.67   | Hz     |
    // | 228      | Input power 1         | 12       | int16  | 21 (*) | 10f   | -32768.0 to 32767.0 | VA or W |
    // | 228      | Input power 2         | 13       | int16  | 23     | 10f   | -32768.0 to 32767.0 | VA or W |
    // | 228      | Input power 3         | 14       | int16  | 25     | 10f   | -32768.0 to 32767.0 | VA or W |
    // =========================================================================================================

    // assumption: all phases have  the same frequency - so pick the first one (valid for 3-phase systems and 1-phase systems)
    this->_frequency = parseSignedorUnsignedInt16(response, 15, 0.01f, true);
    // assumption: all phases have nearly the same voltage - so pick the first one (valid for 3-phase systems and 1-phase systems)
    this->_voltage = parseSignedorUnsignedInt16(response, 3, 0.1f, false);
    // aggregate the current of all phases to get the total current in/out
    this->_current = parseSignedorUnsignedInt16(response, 9, 0.1f, true) + parseSignedorUnsignedInt16(response, 11, 0.1f, true) + parseSignedorUnsignedInt16(response, 13, 0.1f, true);
    // aggregate the power of all phases to get the total power in/out
    this->_power = parseSignedorUnsignedInt16(response, 21, 10.0f, true) + parseSignedorUnsignedInt16(response, 23, 10.0f, true) + parseSignedorUnsignedInt16(response, 25, 10.0f, true);

    if (_callback) {
      _callback(EventType::EVT_READ);
    }
  });

  _client->onErrorHandler([this](Error error, uint32_t token) {
    // ModbusError wraps the error code and provides a readable error message for it
    this->_setError(ModbusError(error), token);
  });
}

void Mycila::Victron::end() {
  if (_client) {
    LOGI(TAG, "Disconnecting from Victron Modbus TCP Server");
    _client->disconnect();
    delete _client;
    _client = nullptr;
    _lastError = "";
    _frequency = NAN;
    _current = NAN;
    _power = NAN;
    _voltage = NAN;
  }
}

void Mycila::Victron::read() {
  if (_client) {
    // Create request for
    // - server ID = 228
    // - function code = 0x03 (read holding register)
    // - start address to read = word 3
    // - number of words to read = 2 (1 word = 2 bytes, therefore int16_t = 1 word)
    // we read more than one register in one batch to save time, the VICTRON inverter has a lot of data to read
    // - REG : 3 Input voltage phase 1
    // - REG : 4 Input voltage phase 2
    // - REG : 5 Input voltage phase 3
    // - REG : 6 Input current phase 1
    // - REG : 7 Input current phase 2
    // - REG : 8 Input current phase 3
    // - REG : 9 Input frequency phase 1
    // - REG : 10 Input frequency phase 2
    // - REG : 11 Input frequency phase 3
    // - REG : 12 Input power power phase 1
    // - REG : 13 Input power power phase 2
    // - REG : 14 Input power power phase 3

    // The request will be issued in the background task, with 12 words to read in a raw
    // - token to match the response with the request. We take the current millis() value for it.
    //
    // If something is missing or wrong with the call parameters, we will immediately get an error code
    // and the request will not be issued
    uint32_t token = millis();
    Error err = _client->addRequest(token, 228, READ_HOLD_REGISTER, 3, 12);
    if (err != SUCCESS) {
      _setError(ModbusError(err), token);
    }
  }
}

void Mycila::Victron::_setError(ModbusError&& error, uint32_t token) {
  std::string msg;
  msg.reserve(128);
  msg.clear();
  msg = "Error ";
  msg += std::to_string((int)error); // NOLINT
  msg += ": ";
  msg += (const char*)error; // NOLINT
  msg += ", token: ";
  msg += std::to_string(token);

  _lastError = msg;

  if (_callback) {
    _callback(EventType::EVT_ERROR);
  }
}
