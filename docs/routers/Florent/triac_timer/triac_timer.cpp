#include "triac_timer.hpp"

#include "driver/gpio.h"
#include "hal/gpio_hal.h"
#include "hal/gpio_ll.h"
#include "hal/gpio_types.h"
#include "soc/gpio_reg.h"

#include "driver/timer.h"
#include "hal/timer_ll.h"
#include "soc/timer_group_reg.h"

#include "esp_check.h"
#include "hal/clk_tree_hal.h"
#include "rom/ets_sys.h"
#include "soc/soc.h"


static const char* TAG = "triac-timer";


// Triac pulse width, microseconds.
#define PULSE_WIDTH_US  ( 400 )

// Minimum delay to reach the voltage required for a gate current of 30mA.
// delay_us = asin((gate_resistor * gate_current) / grid_volt_max) / pi * period_us
// delay_us = asin((330 * 0.03) / 325) / pi * 10000 = 97us
#define PHASE_DELAY_MIN_US  ( 90 )

// Minimum time to lock-out the zero crossing once it is triggered, microseconds.
// This is to avoid interrupts on the opposite edge when there's a significant slew rate.
#define ZC_FILTER_US  ( 2000 )

// Hardware timer
static_assert(CONFIG_TRIAC_TIMER_NUM <= SOC_TIMER_GROUP_TOTAL_TIMERS);
#define TIMER_GRP  ( CONFIG_TRIAC_TIMER_NUM / SOC_TIMER_GROUPS )
#define TIMER_IDX  ( CONFIG_TRIAC_TIMER_NUM % SOC_TIMER_GROUP_TIMERS_PER_GROUP )


/*
 * Low level API similar to "hal/gpio_ll.h" and "hal/timer_ll.h"
 */

#define INLINE_ATTR static inline __attribute__((always_inline))

INLINE_ATTR void _gpio_ll_clear_outputs(uint64_t mask) {
	REG_WRITE(GPIO_OUT_W1TC_REG, (uint32_t)mask);  // GPIO0~31
	#if SOC_GPIO_PIN_COUNT > 31
	if ((uint32_t)(mask >> 32))
		REG_WRITE(GPIO_OUT1_W1TC_REG, (uint32_t)(mask >> 32));  // GPIO32~39
	#endif
}

INLINE_ATTR void _gpio_ll_set_outputs(uint64_t mask) {
	REG_WRITE(GPIO_OUT_W1TS_REG, (uint32_t)mask);  // GPIO0~31
	#if SOC_GPIO_PIN_COUNT > 31
	if ((uint32_t)(mask >> 32))
		REG_WRITE(GPIO_OUT1_W1TS_REG, (uint32_t)(mask >> 32));  // GPIO32~39
	#endif
}


#if TIMER_IDX == 0
	#define TIMG_LOAD_REG     TIMG_T0LOAD_REG(TIMER_GRP)
	#define TIMG_LOADHI_REG   TIMG_T0LOADHI_REG(TIMER_GRP)
	#define TIMG_LOADLO_REG   TIMG_T0LOADLO_REG(TIMER_GRP)
	#define TIMG_UPDATE_REG   TIMG_T0UPDATE_REG(TIMER_GRP)
	#define TIMG_LO_REG       TIMG_T0LO_REG(TIMER_GRP)
	#define TIMG_ALARMHI_REG  TIMG_T0ALARMHI_REG(TIMER_GRP)
	#define TIMG_ALARMLO_REG  TIMG_T0ALARMLO_REG(TIMER_GRP)
	#define TIMG_CONFIG_REG   TIMG_T0CONFIG_REG(TIMER_GRP)
	#define TIMG_INT_CLR      TIMG_T0_INT_CLR
#else
	#define TIMG_LOAD_REG     TIMG_T1LOAD_REG(TIMER_GRP)
	#define TIMG_LOADHI_REG   TIMG_T1LOADHI_REG(TIMER_GRP)
	#define TIMG_LOADLO_REG   TIMG_T1LOADLO_REG(TIMER_GRP)
	#define TIMG_UPDATE_REG   TIMG_T1UPDATE_REG(TIMER_GRP)
	#define TIMG_LO_REG       TIMG_T1LO_REG(TIMER_GRP)
	#define TIMG_ALARMHI_REG  TIMG_T1ALARMHI_REG(TIMER_GRP)
	#define TIMG_ALARMLO_REG  TIMG_T1ALARMLO_REG(TIMER_GRP)
	#define TIMG_CONFIG_REG   TIMG_T1CONFIG_REG(TIMER_GRP)
	#define TIMG_INT_CLR      TIMG_T1_INT_CLR
#endif

INLINE_ATTR void _timer_ll_clear_intr_status() {
	REG_WRITE(TIMG_INT_CLR_TIMERS_REG(TIMER_GRP), TIMG_INT_CLR);
}

INLINE_ATTR void _timer_ll_set_reload_value(uint32_t value) {
	REG_WRITE(TIMG_LOADHI_REG, 0);
	REG_WRITE(TIMG_LOADLO_REG, value);
}

INLINE_ATTR void _timer_ll_set_alarm_value(uint32_t value) {
	REG_WRITE(TIMG_ALARMHI_REG, 0);
	REG_WRITE(TIMG_ALARMLO_REG, value);
}

INLINE_ATTR void _timer_ll_reload() {
	REG_WRITE(TIMG_LOAD_REG, 1);
}

INLINE_ATTR uint32_t _timer_ll_get_value() {
	REG_WRITE(TIMG_UPDATE_REG, 1);
	// Spin wait for the update as implemented by the SDK.
	// We only need the lower 32bits so the wait could be useless if the lower registry is updated first.
	while (REG_READ(TIMG_UPDATE_REG)) { }
	return REG_READ(TIMG_LO_REG);
}

INLINE_ATTR void _timer_ll_set_alarm(uint32_t value) {
	REG_WRITE(TIMG_ALARMLO_REG, value);
	REG_SET_BIT(TIMG_CONFIG_REG, TIMG_T0_ALARM_EN | TIMG_T0_LEVEL_INT_EN);
}

INLINE_ATTR uint32_t _timer_ll_get_alarm() {
	return REG_READ(TIMG_ALARMLO_REG);
}


/**
 *	AC phase delay table (sine square inverse CDF).
 *	power_ratio = sin(2 * pi * delay_ratio) / (2 * pi) - delay_ratio + 1
 *	index 0  ≈   0% power ≈  5% phase angle ≈ 95% firing delay
 *	index 39 ≈  50% power ≈ 50% phase angle ≈ 50% firing delay
 *	index 79 ≈ 100% power ≈ 95% phase angle ≈  5% firing delay
 */

static_assert(CONFIG_TRIAC_RESOLUTION <= 15);

#define TABLE_PHASE_LEN    ( 80U )
#define TABLE_PHASE_SCALE  ( (TABLE_PHASE_LEN - 1U) * (1UL << (16 - CONFIG_TRIAC_RESOLUTION)) )

static const uint16_t TABLE_PHASE_DELAY[TABLE_PHASE_LEN] {
    0xefea, 0xdfd4, 0xd735, 0xd10d, 0xcc12, 0xc7cc, 0xc403, 0xc094,
    0xbd6a, 0xba78, 0xb7b2, 0xb512, 0xb291, 0xb02b, 0xaddc, 0xaba2,
    0xa97a, 0xa762, 0xa557, 0xa35a, 0xa167, 0x9f7f, 0x9da0, 0x9bc9,
    0x99fa, 0x9831, 0x966e, 0x94b1, 0x92f9, 0x9145, 0x8f95, 0x8de8,
    0x8c3e, 0x8a97, 0x88f2, 0x8750, 0x85ae, 0x840e, 0x826e, 0x80cf,
    0x7f31, 0x7d92, 0x7bf2, 0x7a52, 0x78b0, 0x770e, 0x7569, 0x73c2,
    0x7218, 0x706b, 0x6ebb, 0x6d07, 0x6b4f, 0x6992, 0x67cf, 0x6606,
    0x6437, 0x6260, 0x6081, 0x5e99, 0x5ca6, 0x5aa9, 0x589e, 0x5686,
    0x545e, 0x5224, 0x4fd5, 0x4d6f, 0x4aee, 0x484e, 0x4588, 0x4296,
    0x3f6c, 0x3bfd, 0x3834, 0x33ee, 0x2ef3, 0x28cb, 0x202c, 0x1016};


static uint16_t lookup_phase_delay(uint32_t duty, uint16_t period)
{
    uint32_t slot  = duty * TABLE_PHASE_SCALE + (TABLE_PHASE_SCALE >> 1);
    uint16_t index = slot >> 16;
    uint32_t a     = TABLE_PHASE_DELAY[index];
    uint32_t b     = TABLE_PHASE_DELAY[index + 1];
    uint32_t delay = a - (((a - b) * (slot & 0xffff)) >> 16);  // interpolate a b

    return (delay * period) >> 16;  // scale to period
}



static Triac*            _triac_root = NULL;         // instance chaining
static uint64_t          _out_mask   = 0;            // mask for all the output pins
static gpio_num_t        _zc_pin     = GPIO_NUM_NC;  // zero crossing pin number
static uint32_t          _zc_delay   = 0;            // zero crossing delay, unit: 1us
static volatile uint32_t _zc_period  = 0;            // zero crossing period, unit: 1us << 16


static void IRAM_ATTR __isr_crossing(void *arg)
{
	uint32_t tick_now = _timer_ll_get_value();
	uint32_t tick_next = -1;

	// suspend zero-crossing interrupts to lock-out the ZC input
	// uint32_t tick_next = ZC_FILTER_US;
	// gpio_ll_set_intr_type(&GPIO, _zc_pin, GPIO_INTR_DISABLE);

	if (tick_now > ZC_FILTER_US)  // lock-out interval to avoid interrupts on the opposite edge
	{
		// reset timer and clear interrupts in case some are pending
		_timer_ll_reload();
		_timer_ll_clear_intr_status();

		// turn off outputs in case some are still ON
		_gpio_ll_clear_outputs(_out_mask);

		// set each triac firing delay and pick the lowest for the alarm
		for (Triac* triac = _triac_root; triac; triac = triac->_next)
		{
			uint32_t delay = triac->_delay;
			triac->_alarm = delay;
			triac->_triggered = false;

			if (delay < tick_next)
				tick_next = delay;
		}

		// set alarm
		if ((int32_t)tick_next >= 0) {
			_timer_ll_set_alarm(tick_next);
		}

		// update period, Exponential Moving Average N=16
		int32_t avg = _zc_period;  // 1us << 16
		avg += ((int32_t)(tick_now << 16) - avg) >> 4;
		_zc_period = avg;
	}
}


static void IRAM_ATTR __isr_timer(void *arg)
{
	// Clear the interrupt (re-entrance occur when called at the end).
    _timer_ll_clear_intr_status();

	// Get elapsed time since zero crossing. Can use the timer value or alarm value.
	// The alarm value is cheaper but less accurate if the interrupt is delayed.
	uint32_t tick_now  = _timer_ll_get_value();
	// uint32_t tick_now = _timer_ll_get_alarm();
	uint32_t tick_next = -1;

	// ets_printf("%lu\n", tick_now);

	// handle triacs with a lower alarm
	for (Triac* triac = _triac_root; triac; triac = triac->_next)
	{
		uint32_t tick = triac->_alarm; // pulse ON or OFF delay from ZC

		if (tick <= tick_now) {
			// turn OFF pulse if ON
			if (triac->_triggered) {
				_gpio_ll_clear_outputs(triac->_mask);
				triac->_alarm = -1;  // turn off triac alarm
				continue;
			}
			// turn ON pulse
			_gpio_ll_set_outputs(triac->_mask);
			tick = tick_now + PULSE_WIDTH_US;  // add pulse width
			triac->_triggered = true;  // triggered flag
			triac->_alarm = tick;      // triac next alarm
		}

		// pick lowest tick for next alarm
		if (tick < tick_next) {
			tick_next = tick;
		}
	}

	// set alarm, works even when set before the timer current value.
	if ((int32_t)tick_next >= 0) {
		_timer_ll_set_alarm(tick_next);
	}

	// restore zero-crossing interrupts
	// gpio_ll_set_intr_type(&GPIO, _zc_pin, GPIO_INTR_POSEDGE);
}


bool Triac::begin(gpio_num_t sync_pin, uint16_t delay_us, bool invert)
{
	_zc_delay = delay_us;
	_zc_pin   = sync_pin;

	// output pins

	for (Triac* triac = _triac_root; triac; triac = triac->_next)
	{
		ESP_ERROR_CHECK(gpio_set_level(triac->_pin, 0));
		ESP_ERROR_CHECK(gpio_reset_pin(triac->_pin));
		ESP_ERROR_CHECK(gpio_set_direction(triac->_pin, GPIO_MODE_OUTPUT));
	}

	// general purpose timer

	timer_config_t timer_cfg = { };
	timer_cfg.alarm_en    = TIMER_ALARM_DIS;
	timer_cfg.counter_en  = TIMER_START;
	timer_cfg.intr_type   = TIMER_INTR_LEVEL;
	timer_cfg.counter_dir = TIMER_COUNT_UP;
	timer_cfg.auto_reload = TIMER_AUTORELOAD_DIS;
	timer_cfg.clk_src     = TIMER_SRC_CLK_DEFAULT;
	timer_cfg.divider     = clk_hal_apb_get_freq_hz() / 1000000;  // 1us tick

	ESP_ERROR_CHECK(timer_init((timer_group_t)TIMER_GRP,
	                           (timer_idx_t)TIMER_IDX,
							   &timer_cfg));

	ESP_ERROR_CHECK(timer_isr_register((timer_group_t)TIMER_GRP,
	                           		   (timer_idx_t)TIMER_IDX,
									   __isr_timer,
									   NULL,
									   ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM,
									   NULL));

	ESP_ERROR_CHECK(timer_enable_intr((timer_group_t)TIMER_GRP,
	                                  (timer_idx_t)TIMER_IDX));

	_timer_ll_set_reload_value(0);
	_timer_ll_set_alarm_value(0);

	// zero-crossing interrupt

	ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM));

    gpio_config_t io_cfg = { };
    io_cfg.intr_type    = invert ? GPIO_INTR_NEGEDGE : GPIO_INTR_POSEDGE;
    io_cfg.mode         = GPIO_MODE_INPUT;
    io_cfg.pin_bit_mask = (1ULL << sync_pin);
    io_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_cfg.pull_up_en   = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_cfg));

    ESP_ERROR_CHECK(gpio_isr_handler_add(sync_pin, __isr_crossing, NULL));

	// esp_intr_dump(NULL);

	return true;
}


void Triac::end()
{
	gpio_isr_handler_remove(_zc_pin);
	timer_deinit((timer_group_t)TIMER_GRP, (timer_idx_t)TIMER_IDX);
	_gpio_ll_clear_outputs(_out_mask);  // turn off triacs
}


uint32_t Triac::get_period_us()
{
	return (_zc_period + (1U << 14)) >> 15;  // shift 16, x2, round
}



Triac::Triac(gpio_num_t pin) :
	_next(_triac_root),  // chain next instance
	_pin(pin),           // output pin number
	_mask(1ULL << pin),  // output pin mask
	_delay(-1),          // triac firing tick
	_alarm(-1),          // next tick for pulse start or pulse end
	_triggered(false)    // triggered flag
{
	_triac_root = this;  // chain as root
	_out_mask |= _mask;  // add to outputs mask
}


void Triac::set(int32_t value)
{
	if (value <= 0) {
		_delay = UINT32_MAX;  // OFF 0%
		this->value = 0;
	}
	else if (value >= TRIAC_MAX) {
		_delay = _zc_delay + PHASE_DELAY_MIN_US;  // ON 100%
		this->value = TRIAC_MAX;
	}
	else {
		_delay = _zc_delay + lookup_phase_delay(value, _zc_period >> 16);  // dimmed
		this->value = value;
	}
}


int32_t Triac::add(int32_t value)
{
    int32_t v = this->value;
    set(v + value);
    return this->value - v;
}


void Triac::test_zc()
{
    _delay = _zc_delay;
}
