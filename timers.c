#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "app_error.h"
#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "common.h"

#define BTN_POLL_INTERVAL                   APP_TIMER_TICKS(10)           // Poll each button every 10ms
#define LED_BLINK_INTERVAL                  APP_TIMER_TICKS(700)          // Advertising mode flash time
#define FAST_LED_BLINK_INTERVAL             APP_TIMER_TICKS(100)          // Pair mode flash time
#define INACTIVE_TIME                       APP_TIMER_TICKS(300000)       // sleep after 5min of inactivity
#define BATTERY_LEVEL_MEAS_INTERVAL         APP_TIMER_TICKS(3000)         // Battery measurement interval


// Timer Objects
APP_TIMER_DEF(m_battery_timer_id);      // Battery Level Measure Timer
APP_TIMER_DEF(m_btn_poll_timer_id);     // Poll buttons for changes
APP_TIMER_DEF(m_led_blink_timer_id);    // Flashing LED toggle timer
APP_TIMER_DEF(m_inactive_timer_id);     // Inactive sleep timer


// reset the count down until the sleep timer is triggered
void extend_inactive_timer()
{
    ret_code_t err_code;
	
	err_code = app_timer_stop(m_inactive_timer_id);
    APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(m_inactive_timer_id, INACTIVE_TIME, NULL);
	APP_ERROR_CHECK(err_code);
}


static void led_blink_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
	toggle_led();
}


static void inactive_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
	NRF_LOG_INFO("Inactive Timeout: sleeping");
	set_device_state(SLEEP);
}


static void btn_poll_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
	poll_keys();
}


static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
	battery_level_update(get_battery_level());
}


void start_led_flash_timer(bool fast)
{
    ret_code_t err_code;
	
	if (fast)
	{
		err_code = app_timer_start(m_led_blink_timer_id, FAST_LED_BLINK_INTERVAL, NULL);
	}
	else
	{
		err_code = app_timer_start(m_led_blink_timer_id, LED_BLINK_INTERVAL, NULL);
	}
	APP_ERROR_CHECK(err_code);
}


void stop_led_flash_timer(void)
{
    ret_code_t err_code;
	err_code = app_timer_stop(m_led_blink_timer_id);
	APP_ERROR_CHECK(err_code);
}


void timers_start(void)
{
    ret_code_t err_code;

	// start checking battery level
    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

	// start polling buttons
	err_code = app_timer_start(m_btn_poll_timer_id, BTN_POLL_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);

	// start the inactivity timer
	extend_inactive_timer();
}


void timers_init(void)
{
    ret_code_t err_code;

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_battery_timer_id,
								APP_TIMER_MODE_REPEATED, battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&m_btn_poll_timer_id,
								APP_TIMER_MODE_REPEATED, btn_poll_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_led_blink_timer_id,
								APP_TIMER_MODE_REPEATED, led_blink_timeout_handler);
    APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&m_inactive_timer_id,
								APP_TIMER_MODE_REPEATED, inactive_timeout_handler);
    APP_ERROR_CHECK(err_code);
}
