// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaDimmerZeroCross.h>

// lock
#include <freertos/FreeRTOS.h>

// gpio
#include <driver/gpio.h>
#include <esp32-hal-gpio.h>
#include <hal/gpio_ll.h>
#include <soc/gpio_struct.h>

// timers
#include <inlined_gptimer.h>

// logging
#include <esp32-hal-log.h>

#ifdef MYCILA_PULSE_DEBUG
  #include <rom/ets_sys.h>
#endif

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

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

#ifndef GPIO_IS_VALID_GPIO
  #define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

// Minimum delay to reach the voltage required for a gate current of 30mA.
// delay_us = asin((gate_resistor * gate_current) / grid_volt_max) / pi * period_us
// delay_us = asin((330 * 0.03) / 325) / pi * 10000 = 97us
#define PHASE_DELAY_MIN_US (90)

#define TAG "ZC_DIMMER"

Mycila::ZeroCrossDimmer* Mycila::ZeroCrossDimmer::_dimmers[2] = {nullptr, nullptr};
size_t Mycila::ZeroCrossDimmer::_dimmerCount = 0;
gptimer_handle_t Mycila::ZeroCrossDimmer::_fireTimer = nullptr;
portMUX_TYPE Mycila::ZeroCrossDimmer::_spinlock = portMUX_INITIALIZER_UNLOCKED;

void Mycila::ZeroCrossDimmer::begin() {
  if (_enabled)
    return;

  if (!GPIO_IS_VALID_OUTPUT_GPIO(_pin)) {
    LOGE(TAG, "Disable ZC Dimmer: Invalid pin: %" PRId8, _pin);
    return;
  }

  LOGI(TAG, "Enable Zero-Cross Dimmer on pin %" PRId8, _pin);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  _enabled = _registerDimmer(this);

  // restart with last saved value
  setDutyCycle(_dutyCycle);
}

void Mycila::ZeroCrossDimmer::end() {
  if (!_enabled)
    return;
  LOGI(TAG, "Disable ZC Dimmer on pin %" PRId8, _pin);
  _enabled = false;
  _unregisterDimmer(this);
  // Note: do not set _dutyCycle to 0 in order to keep last set user value
  _delay = UINT16_MAX;
  digitalWrite(_pin, LOW);
}

bool Mycila::ZeroCrossDimmer::apply() {
  if (_delay < PHASE_DELAY_MIN_US)
    _delay = PHASE_DELAY_MIN_US;
  return true;
}

bool Mycila::ZeroCrossDimmer::_registerDimmer(ZeroCrossDimmer* dimmer) {
  portENTER_CRITICAL_SAFE(&_spinlock);

  // no slot left ?
  if (_dimmerCount >= MYCILA_DIMMER_MAX_COUNT) {
    portEXIT_CRITICAL_SAFE(&_spinlock);
    ESP_LOGE(TAG, "No dimmer slot left: increase MYCILA_DIMMER_MAX_COUNT");
    return false;
  }

  for (size_t i = 0; i < MYCILA_DIMMER_MAX_COUNT; i++) {
    // find a free slot ?
    if (_dimmers[i] == nullptr) {
      _dimmers[i] = dimmer;
      _dimmerCount++;

      if (_dimmerCount == 1) {
        portEXIT_CRITICAL_SAFE(&_spinlock);
        ESP_LOGD(TAG, "First dimmer %p on pin %d registered: start firing timer", dimmer, dimmer->getPin());

        // first dimmer added, start timer
        gptimer_config_t timer_config;
        timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
        timer_config.direction = GPTIMER_COUNT_UP;
        timer_config.resolution_hz = 1000000; // 1MHz resolution
        timer_config.flags.intr_shared = true;
        timer_config.intr_priority = 0;

        ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &_fireTimer));
        gptimer_event_callbacks_t callbacks_config;
        callbacks_config.on_alarm = _fireTimerISR;
        ESP_ERROR_CHECK(gptimer_register_event_callbacks(_fireTimer, &callbacks_config, nullptr));
        ESP_ERROR_CHECK(gptimer_enable(_fireTimer));
        ESP_ERROR_CHECK(inlined_gptimer_start(_fireTimer));

        return true;

      } else {
        portEXIT_CRITICAL_SAFE(&_spinlock);
        ESP_LOGD(TAG, "New dimmer %p on pin %d registered", dimmer, dimmer->getPin());
        return true;
      }
    }
  }

  portEXIT_CRITICAL_SAFE(&_spinlock);
  ESP_LOGE(TAG, "No free slot found for dimmer %p on pin %d", dimmer, dimmer->getPin());
  return false;
}

void Mycila::ZeroCrossDimmer::_unregisterDimmer(ZeroCrossDimmer* dimmer) {
  portENTER_CRITICAL_SAFE(&_spinlock);

  if (_dimmerCount == 0) {
    portEXIT_CRITICAL_SAFE(&_spinlock);
    return;
  }

  for (size_t i = 0; i < MYCILA_DIMMER_MAX_COUNT; i++) {
    if (_dimmers[i] == dimmer) {
      _dimmers[i] = nullptr;
      _dimmerCount--;

      if (_dimmerCount == 0) {
        portEXIT_CRITICAL_SAFE(&_spinlock);
        ESP_LOGD(TAG, "Last dimmer %p on pin %d unregistered: stop firing timer", dimmer, dimmer->getPin());

        // last dimmer removed, stop timer
        inlined_gptimer_stop(_fireTimer); // might be already stopped
        ESP_ERROR_CHECK(gptimer_disable(_fireTimer));
        ESP_ERROR_CHECK(gptimer_del_timer(_fireTimer));
        _fireTimer = nullptr;
        return;

      } else {
        portEXIT_CRITICAL_SAFE(&_spinlock);
        ESP_LOGD(TAG, "Dimmer %p on pin %d unregistered", dimmer, dimmer->getPin());
        return;
      }
    }
  }

  portEXIT_CRITICAL_SAFE(&_spinlock);
  ESP_LOGE(TAG, "Dimmer %p on pin %d not found", dimmer, dimmer->getPin());
}

bool ARDUINO_ISR_ATTR Mycila::ZeroCrossDimmer::_fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg) {
  for (size_t i = 0; i < MYCILA_DIMMER_MAX_COUNT; i++) {
    ZeroCrossDimmer* dimmer = _dimmers[i];
    // if (dimmer) {
    //   if (dimmer->_delay == UINT16_MAX) {
    //     digitalWrite(dimmer->_pin, LOW);
    //   } else {
    //     digitalWrite(dimmer->_pin, HIGH);
    //     ESP_ERROR_CHECK(inlined_gptimer_set_alarm_action(_fireTimer, dimmer->_delay));
    //   }
    // }
  }
  return false;
}

void ARDUINO_ISR_ATTR Mycila::ZeroCrossDimmer::onZeroCross(int16_t delayUntilZero, void* arg) {
  ESP_ERROR_CHECK(inlined_gptimer_set_raw_count(_fireTimer, 0));

  // TODO: consider MYCILA_PULSE_ZC_SHIFT_US

  // for (size_t i = 0; i < MYCILA_DIMMER_MAX_COUNT; i++) {
  //   Dimmer* dimmer = _dimmers[i];
  //   if (dimmer) {
  //     if (dimmer->_delay == UINT16_MAX) {
  //       digitalWrite(dimmer->_pin, LOW);
  //     } else {
  //       digitalWrite(dimmer->_pin, HIGH);
  //       ESP_ERROR_CHECK(inlined_gptimer_set_alarm_action(_fireTimer, dimmer->_delay));
  //     }
  //   }
  // }
}
