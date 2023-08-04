// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaDisplay.h>
#include <MycilaLogger.h>

#include <assert.h>

#define TAG "DISPLAY"

#define DISPLAY_FONT u8g2_font_5x8_tr
#define DISPLAY_LINE_HEIGHT_PX 9

static const char* DisplayTypeNames[] = {"SH1106", "SH1107", "SSD1306"};

void Mycila::DisplayClass::begin() {
  if (_enabled)
    return;

  if (!DisplayConfig.isEnabled())
    return;

  int32_t clkPin = DisplayConfig.getClockPin();
  if (GPIO_IS_VALID_OUTPUT_GPIO(clkPin)) {
    _clkPin = (gpio_num_t)clkPin;
  } else {
    Logger.error(TAG, "Invalid Clock pin: %u", _clkPin);
    _clkPin = GPIO_NUM_NC;
    return;
  }

  int32_t dataPin = DisplayConfig.getDataPin();
  if (GPIO_IS_VALID_OUTPUT_GPIO(dataPin)) {
    _dataPin = (gpio_num_t)dataPin;
  } else {
    Logger.error(TAG, "Invalid Data pin: %u", _dataPin);
    _dataPin = GPIO_NUM_NC;
    return;
  }

  DisplayType type = DisplayConfig.getType();

  Logger.info(TAG, "Enable Display...");
  Logger.debug(TAG, "- Type: %s", DisplayTypeNames[static_cast<int>(type)]);
  Logger.debug(TAG, "- Clock Pin: %u", _clkPin);
  Logger.debug(TAG, "- Data Pin: %u", _dataPin);
  Logger.debug(TAG, "- Rotation: %u°", DisplayConfig.getRotation());
  Logger.debug(TAG, "- Power Save Delay: %u seconds", DisplayConfig.getPowerSaveDelay());

  switch (type) {
    case DisplayType::SSD1306:
      switch (DisplayConfig.getRotation()) {
        case 90:
          _display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R1, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        case 180:
          _display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R2, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        case 270:
          _display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R3, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        default:
          _display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
      }
      break;
    case DisplayType::SH1106:
      switch (DisplayConfig.getRotation()) {
        case 90:
          _display = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R1, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        case 180:
          _display = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R2, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        case 270:
          _display = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R3, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        default:
          _display = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
      }
      break;
    case DisplayType::SH1107:
      switch (DisplayConfig.getRotation()) {
        case 90:
          _display = new U8G2_SH1107_64X128_F_HW_I2C(U8G2_R2, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        case 180:
          _display = new U8G2_SH1107_64X128_F_HW_I2C(U8G2_R3, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        case 270:
          _display = new U8G2_SH1107_64X128_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
        default:
          _display = new U8G2_SH1107_64X128_F_HW_I2C(U8G2_R1, U8X8_PIN_NONE, _clkPin, _dataPin);
          break;
      }
      break;
    default:
      assert(false);
      break;
  }

  _display->begin();
  _display->setPowerSave(false);
  _display->setFont(DISPLAY_FONT);

  _enabled = true;
  _active = true;
}

void Mycila::DisplayClass::end() {
  if (!_enabled)
    return;
  Logger.info(TAG, "Disable Display...");
  _enabled = false;
  _active = false;
  _display->clear();
  delete _display;
  _dataPin = GPIO_NUM_NC;
  _clkPin = GPIO_NUM_NC;
}

void Mycila::DisplayClass::loop() {
  if (!_enabled)
    return;

  // readjust live power save state
  const uint32_t delay = DisplayConfig.getPowerSaveDelay();
  _powerSave.setEnable(delay > 0, false);

  if (_active && delay > 0 && _powerSave.isAllowed(delay)) {
    Logger.info(TAG, "Display power saving...");
    _display->setPowerSave(true);
    _active = false;

  } else if (!_active && delay == 0) {
    Logger.info(TAG, "Display power saving disabled");
    _display->setPowerSave(false);
    _active = true;
  }
}

void Mycila::DisplayClass::setActive(bool active) {
  if (!_enabled)
    return;

  if (active == _active)
    return;

  if (active && !_active) {
    _display->setPowerSave(false);
    _active = true;
  } else if (!active && _active) {
    _display->setPowerSave(true);
    _active = false;
  }
}

void Mycila::DisplayClass::print(const char lines[MYCILA_DISPLAY_LINE_COUNT][MYCILA_DISPLAY_LINE_LENGTH]) {
  if (!_enabled)
    return;

  bool changed = false;
  for (int i = 0; i < MYCILA_DISPLAY_LINE_COUNT; i++) {
    if (strcmp(_lines[i], lines[i]) != 0) {
      snprintf(_lines[i], MYCILA_DISPLAY_LINE_LENGTH, "%s", lines[i]);
      changed = true;
    }
  }

  if (changed) {
    _display->clearBuffer();
    _display->drawStr(1, DISPLAY_LINE_HEIGHT_PX, _lines[0]);
    _display->drawStr(1, 2 * DISPLAY_LINE_HEIGHT_PX + 1, _lines[1]);
    _display->drawStr(1, 3 * DISPLAY_LINE_HEIGHT_PX + 2, _lines[2]);
    _display->drawStr(1, 4 * DISPLAY_LINE_HEIGHT_PX + 3, _lines[3]);
    _display->drawStr(1, 5 * DISPLAY_LINE_HEIGHT_PX + 4, _lines[4]);
    _display->drawStr(1, 6 * DISPLAY_LINE_HEIGHT_PX + 5, _lines[5]);
    _display->sendBuffer();

    if (!_active)
      _display->setPowerSave(false);

    _active = true;
    _powerSave.touch();
  }
}

namespace Mycila {
  DisplayClass Display;
  DisplayConfigClass DisplayConfig;
} // namespace Mycila
