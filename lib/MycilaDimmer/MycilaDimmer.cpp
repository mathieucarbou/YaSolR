// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <MycilaDimmer.h>

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

#define TAG "DIMMER"

// Minimum delay to reach the voltage required for a gate current of 30mA.
// delay_us = asin((gate_resistor * gate_current) / grid_volt_max) / pi * period_us
// delay_us = asin((330 * 0.03) / 325) / pi * 10000 = 97us
#define PHASE_DELAY_MIN_US (90)

#define TABLE_PHASE_LEN (80U)

static const uint32_t DIMMER_MAX = (1 << MYCILA_DIMMER_RESOLUTION) - 1;
static const uint32_t TABLE_PHASE_SCALE = (TABLE_PHASE_LEN - 1U) * (1UL << (16 - MYCILA_DIMMER_RESOLUTION));
static const uint16_t TABLE_PHASE_DELAY[TABLE_PHASE_LEN] PROGMEM = {0xefea, 0xdfd4, 0xd735, 0xd10d, 0xcc12, 0xc7cc, 0xc403, 0xc094, 0xbd6a, 0xba78, 0xb7b2, 0xb512, 0xb291, 0xb02b, 0xaddc, 0xaba2, 0xa97a, 0xa762, 0xa557, 0xa35a, 0xa167, 0x9f7f, 0x9da0, 0x9bc9, 0x99fa, 0x9831, 0x966e, 0x94b1, 0x92f9, 0x9145, 0x8f95, 0x8de8, 0x8c3e, 0x8a97, 0x88f2, 0x8750, 0x85ae, 0x840e, 0x826e, 0x80cf, 0x7f31, 0x7d92, 0x7bf2, 0x7a52, 0x78b0, 0x770e, 0x7569, 0x73c2, 0x7218, 0x706b, 0x6ebb, 0x6d07, 0x6b4f, 0x6992, 0x67cf, 0x6606, 0x6437, 0x6260, 0x6081, 0x5e99, 0x5ca6, 0x5aa9, 0x589e, 0x5686, 0x545e, 0x5224, 0x4fd5, 0x4d6f, 0x4aee, 0x484e, 0x4588, 0x4296, 0x3f6c, 0x3bfd, 0x3834, 0x33ee, 0x2ef3, 0x28cb, 0x202c, 0x1016};

Mycila::Dimmer* Mycila::Dimmer::_dimmers[MYCILA_DIMMER_MAX_COUNT] = {nullptr};
size_t Mycila::Dimmer::_dimmerCount = 0;
gptimer_handle_t Mycila::Dimmer::_fireTimer = nullptr;
portMUX_TYPE Mycila::Dimmer::_spinlock = portMUX_INITIALIZER_UNLOCKED;

void Mycila::Dimmer::begin(int8_t pin, uint16_t semiPeriod) {
  if (_enabled)
    return;

  if (!semiPeriod) {
    LOGE(TAG, "Disable Dimmer on pin %" PRId8 ": Invalid semi-period: %" PRIu16 " us", pin, semiPeriod);
    return;
  }

  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    LOGE(TAG, "Disable Dimmer: Invalid pin: %" PRId8, pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  LOGI(TAG, "Enable Dimmer on pin %" PRId8 " with semi-period %" PRIu16 " us", pin, semiPeriod);

  _semiPeriod = semiPeriod;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);

  _enabled = _registerDimmer(this);

  // restart with last saved value
  setDutyCycle(_dutyCycle);
}

void Mycila::Dimmer::end() {
  if (!_enabled)
    return;
  _unregisterDimmer(this);
  _enabled = false;
  LOGI(TAG, "Disable Dimmer on pin %" PRId8, _pin);
  // Note: do not set _dutyCycle to 0 in order to keep last set user value
  _delay = UINT16_MAX;
  _semiPeriod = 0;
  digitalWrite(_pin, LOW);
  _pin = GPIO_NUM_NC;
}

void Mycila::Dimmer::setDutyCycle(float newDutyCycle) {
  // apply limit and save the wanted duty cycle.
  // it wil only be applied when dimmer will be on
  _dutyCycle = constrain(newDutyCycle, 0, _dutyCycleLimit);

  if (!_enabled)
    return;

  // duty remapping (equivalent to Shelly Dimmer remapping feature)
  const float mappedDutyCycle = _dutyCycleMin + _dutyCycle * (_dutyCycleMax - _dutyCycleMin);

  if (mappedDutyCycle == 0) {
    _delay = UINT16_MAX;
  } else if (mappedDutyCycle == 1) {
    _delay = PHASE_DELAY_MIN_US;
  } else {
    _delay = _lookupPhaseDelay(mappedDutyCycle);
    if (_delay < PHASE_DELAY_MIN_US)
      _delay = PHASE_DELAY_MIN_US;
  }
}

void Mycila::Dimmer::setDutyCycleLimit(float limit) {
  _dutyCycleLimit = constrain(limit, 0, 1);
  LOGD(TAG, "Set dimmer %" PRId8 " duty cycle limit to %f", _pin, _dutyCycleLimit);
  if (_dutyCycle > _dutyCycleLimit)
    setDutyCycle(_dutyCycleLimit);
}

void Mycila::Dimmer::setDutyCycleMin(float min) {
  _dutyCycleMin = constrain(min, 0, _dutyCycleMax);
  LOGD(TAG, "Set dimmer %" PRId8 " duty cycle min to %f", _pin, _dutyCycleMin);
  setDutyCycle(_dutyCycle);
}

void Mycila::Dimmer::setDutyCycleMax(float max) {
  _dutyCycleMax = constrain(max, _dutyCycleMin, 1);
  LOGD(TAG, "Set dimmer %" PRId8 " duty cycle max to %f", _pin, _dutyCycleMax);
  setDutyCycle(_dutyCycle);
}

uint16_t Mycila::Dimmer::_lookupPhaseDelay(float dutyCycle) {
  uint32_t duty = dutyCycle * DIMMER_MAX;
  uint32_t slot = duty * TABLE_PHASE_SCALE + (TABLE_PHASE_SCALE >> 1);
  uint32_t index = slot >> 16;
  uint32_t a = TABLE_PHASE_DELAY[index];
  uint32_t b = TABLE_PHASE_DELAY[index + 1];
  uint32_t delay = a - (((a - b) * (slot & 0xffff)) >> 16); // interpolate a b
  return (delay * _semiPeriod) >> 16;                       // scale to period
}

bool Mycila::Dimmer::_registerDimmer(Dimmer* dimmer) {
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

void Mycila::Dimmer::_unregisterDimmer(Dimmer* dimmer) {
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

bool ARDUINO_ISR_ATTR Mycila::Dimmer::_fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg) {
  for (size_t i = 0; i < MYCILA_DIMMER_MAX_COUNT; i++) {
    Dimmer* dimmer = _dimmers[i];
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

void ARDUINO_ISR_ATTR Mycila::Dimmer::onZeroCross(int16_t delayUntilZero, void* arg) {
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
