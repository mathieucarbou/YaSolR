// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <MycilaFronius.h>

#include <cstring>
#include <memory>
#include <string>
#include <utility>

#define TAG "FRONIUS"

// Parses a SunSpec float32 value (2 registers, big-endian, IEEE-754) from a
// Modbus response. SunSpec "float" model registers (211/212/213 for meters,
// 111/112/113 for inverters) encode each value as 4 bytes spanning 2
// consecutive 16-bit registers, high register first.
static float parseFloat32(const ModbusMessage& response, size_t offset) {
  uint32_t raw = (static_cast<uint32_t>(response[offset])     << 24) |
                 (static_cast<uint32_t>(response[offset + 1]) << 16) |
                 (static_cast<uint32_t>(response[offset + 2]) << 8)  |
                 (static_cast<uint32_t>(response[offset + 3]));
  float value;
  std::memcpy(&value, &raw, sizeof(value)); // avoid strict-aliasing UB
  return value;
}

// SunSpec Meter float model (211/212/213) register map.
// Addresses below are the raw Modbus register numbers as accepted directly
// by this device (confirmed via live scan) — NOT offset by -40001. The
// device's SunSpec base ("SunS" marker) sits at register 40000, the Common
// model (ID 1) runs 40002-40068, and the Meter float model starts at 40069.
namespace FroniusMeterRegisters {
  static constexpr uint16_t ID_ADDR    = 40069; // model ID: 211/212/213 expected
  static constexpr uint16_t A_ADDR     = 40071; // AC Total Current, float32
  static constexpr uint16_t PHV_ADDR   = 40079; // AC Voltage (average), float32
  static constexpr uint16_t HZ_ADDR    = 40095; // AC Frequency, float32
  static constexpr uint16_t W_ADDR     = 40097; // AC Power (total), float32
//  static constexpr uint16_t VA_ADDR    = 40105; // AC Apparent Power (total), float32
//  static constexpr uint16_t PF_ADDR    = 40121; // Power Factor (average), float32
//  static constexpr uint16_t WH_R_ADDR  = 40129; // Energy returned (total), float32
//  static constexpr uint16_t WH_I_ADDR  = 40137; // Energy imported (total), float32

  static constexpr uint16_t READ_START = ID_ADDR;
  // Derived from the last field we actually parse (W_ADDR, a float32 = 2
  // registers) rather than hardcoded, so uncommenting/moving fields above
  // can't silently desync this from what's actually being read.
  static constexpr uint16_t READ_COUNT = (W_ADDR + 2) - READ_START;

  // Candidate device/slave IDs for the SmartMeter — NOT the inverter's ID
  // (often 1). Observed values: 240 on older Datamanager/Symo setups, 200 on
  // GEN24 systems. We probe these in order and lock in whichever responds
  // with a valid meter model ID.
  static constexpr uint8_t CANDIDATE_DEVICE_IDS[] = {240, 200};
  static constexpr size_t  CANDIDATE_COUNT = sizeof(CANDIDATE_DEVICE_IDS) / sizeof(CANDIDATE_DEVICE_IDS[0]);

  // Modbus response layout: [slave ID][function code][byte count][register data...]
  static constexpr size_t RESPONSE_HEADER_SIZE = 3;
  static constexpr size_t BYTES_PER_REGISTER   = 2;

  // Byte offset within the response payload for a given register address.
  static constexpr size_t byteOffset(uint16_t registerAddr) {
    return RESPONSE_HEADER_SIZE + (registerAddr - READ_START) * BYTES_PER_REGISTER;
  }
} // namespace FroniusMeterRegisters

void Mycila::Fronius::begin(const char* host, uint16_t port) {
  if (_client) {
    return;
  }

  ESP_LOGI(TAG, "Connecting to Fronius Modbus TCP Server %s:%" PRIu16 "", host, port);
  _client = std::make_unique<ModbusClientTCPasync>(IPAddress(host), port);
  _client->setTimeout(DEFAULTTIMEOUT);
  _client->setIdleTimeout(DEFAULTIDLETIME);

  // Reset device-ID discovery state on (re)connect.
  _meterDeviceIdIndex = 0;
  _meterDeviceIdConfirmed = false;

  // Callbacks capture a weak_ptr to the alive token, not raw `this`. If
  // end() runs (or this object is destroyed) before a pending response or
  // error arrives, the weak_ptr fails to lock and the callback is a no-op —
  // this avoids touching a torn-down/destroyed object.
  std::weak_ptr<bool> weakAlive = _aliveToken;

  _client->onDataHandler([this, weakAlive](ModbusMessage response, uint32_t token) {
    auto alive = weakAlive.lock();
    if (!alive) {
      return;
    }

    using FroniusMeterRegisters::A_ADDR;
    using FroniusMeterRegisters::byteOffset;
    using FroniusMeterRegisters::HZ_ADDR;
    using FroniusMeterRegisters::ID_ADDR;
    using FroniusMeterRegisters::PHV_ADDR;
    using FroniusMeterRegisters::READ_COUNT;
    using FroniusMeterRegisters::READ_START;
    using FroniusMeterRegisters::W_ADDR;

    // Guard against short/malformed frames before indexing into the buffer.
    const size_t expectedSize = byteOffset(READ_START + READ_COUNT);
    if (response.size() < expectedSize) {
      ESP_LOGW(TAG, "Meter response too short: got %u bytes, expected >= %u",
               static_cast<unsigned>(response.size()), static_cast<unsigned>(expectedSize));
      return;
    }

    // Use byteOffset() rather than hardcoded response[3]/response[4] so
    // this can't silently desync from READ_START if it ever changes.
    uint16_t modelId = (static_cast<uint16_t>(response[byteOffset(ID_ADDR)]) << 8) |
                        response[byteOffset(ID_ADDR) + 1];
    if (modelId != 211 && modelId != 212 && modelId != 213) {
      // Wrong device ID guessed, or device isn't reporting the float model.
      // If we're still probing candidates, this response arriving at all
      // (rather than an error) means the device ID was live but the model
      // wasn't what we expected — try the next candidate.
      ESP_LOGW(TAG, "Unexpected SunSpec meter model ID: %u", modelId);
      if (!_meterDeviceIdConfirmed) {
        _advanceMeterDeviceIdCandidate();
      }
      return;
    }

    if (!_meterDeviceIdConfirmed) {
      _meterDeviceIdConfirmed = true;
      ESP_LOGI(TAG, "Confirmed meter device ID: %u", _currentMeterDeviceId());
    }

    _lastError.clear();

    this->_voltage         = parseFloat32(response, byteOffset(PHV_ADDR));
    this->_frequency       = parseFloat32(response, byteOffset(HZ_ADDR));
    this->_current         = parseFloat32(response, byteOffset(A_ADDR));
    this->_power           = parseFloat32(response, byteOffset(W_ADDR));

    if (_callback) {
      _callback(EventType::EVT_READ);
    }
  });

  _client->onErrorHandler([this, weakAlive](Error error, uint32_t token) {
    auto alive = weakAlive.lock();
    if (!alive) {
      return;
    }

    using FroniusMeterRegisters::CANDIDATE_COUNT;

    // While still probing, an error (e.g. Illegal Data Address / Gateway
    // Target Device Failed to Respond) means this candidate ID was wrong.
    // Advance to the next candidate; the next scheduled read() call will
    // use it. (We deliberately do NOT call read() synchronously from here
    // — retrying from inside this callback would mean a reentrant
    // addRequest() call, and if a device is unreachable that can turn into
    // a tight, uncontrolled retry loop. Letting the normal read() polling
    // cadence pick up the new candidate keeps this predictable.)
    if (!_meterDeviceIdConfirmed && _meterDeviceIdIndex + 1 < CANDIDATE_COUNT) {
      ESP_LOGW(TAG, "Meter device ID %u failed (%s), trying next candidate on next read()",
               _currentMeterDeviceId(), (const char*)ModbusError(error));
      _advanceMeterDeviceIdCandidate();
      return;
    }

    this->_setError(ModbusError(error), token);
  });
}

void Mycila::Fronius::end() {
  if (_client) {
    ESP_LOGI(TAG, "Disconnecting from Fronius Modbus TCP Server");

    // Invalidate the alive token first: any response/error that arrives
    // for an in-flight request after this point will find the weak_ptr
    // captured in the callbacks can no longer be locked, and will safely
    // no-op instead of touching members mid-teardown.
    _aliveToken = std::make_shared<bool>(true);

    _client->disconnect();
    _client.reset();
    _lastError.clear();
    _frequency = NAN;
    _current = NAN;
    _power = NAN;
    _voltage = NAN;
    _meterDeviceIdIndex = 0;
    _meterDeviceIdConfirmed = false;
  }
}

void Mycila::Fronius::read() {
  if (_client) {
    uint32_t token = millis();

    Error err = _client->addRequest(token, _currentMeterDeviceId(), READ_HOLD_REGISTER,
                                     FroniusMeterRegisters::READ_START,
                                     FroniusMeterRegisters::READ_COUNT);
    if (err != SUCCESS) {
      _setError(ModbusError(err), token);
    }
  }
}

uint8_t Mycila::Fronius::_currentMeterDeviceId() const {
  return FroniusMeterRegisters::CANDIDATE_DEVICE_IDS[_meterDeviceIdIndex];
}

void Mycila::Fronius::_advanceMeterDeviceIdCandidate() {
  using FroniusMeterRegisters::CANDIDATE_COUNT;
  if (_meterDeviceIdIndex + 1 < CANDIDATE_COUNT) {
    _meterDeviceIdIndex++;
  } else {
    // Exhausted all candidates; wrap around and let the caller's normal
    // retry/error-reporting cadence keep trying rather than getting stuck.
    _meterDeviceIdIndex = 0;
  }
  // Note: intentionally does NOT trigger an immediate re-read. The next
  // scheduled call to read() (from application code's normal polling loop)
  // will pick up the new candidate device ID. See onErrorHandler above for
  // why this is not done synchronously/recursively.
}

void Mycila::Fronius::_setError(ModbusError&& error, uint32_t token) {
  std::string msg;
  msg.reserve(128);
  msg = "Error ";
  msg += std::to_string((int)error); // NOLINT
  msg += ": ";
  msg += (const char*)error; // NOLINT
  msg += ", token: ";
  msg += std::to_string(token);

  _lastError = std::move(msg);

  if (_callback) {
    _callback(EventType::EVT_ERROR);
  }
}
