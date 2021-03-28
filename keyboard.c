#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "common.h"
#include "codes.h"
#include "usb_hid_keys.h"

#define POWER_KEY_PIN                    NRF_GPIO_PIN_MAP(0, 26)  // D9
#define LEFT_KEY_PIN                     NRF_GPIO_PIN_MAP(0, 6)   // D11
#define RIGHT_KEY_PIN                    NRF_GPIO_PIN_MAP(0, 8)   // D12
#define LED_PIN                          NRF_GPIO_PIN_MAP(0, 27)  // D10

#define MAX_CODE_SIZE                    10                       // max amount of morse code steps (in poll cycles)

#define ADVANCE_TIME                     20                       // time before advancing to the next morse code step
#define KEY_HOLD_TIME                    100                      // time before the key held is repeated
#define KEY_REPEAT_TIME                  10                       // speed the key is repeated

// Debounced state for a button
typedef struct
{
	bool prev;
	bool new;
	bool state;
} key_reading;

static key_reading left_key;
static key_reading right_key;
static key_reading power_key;

enum key_event { NO_CHANGE, PRESSED, RELEASED};

static bool key_repeat_mode = false;             // the key is being repeated
static uint16_t key_hold_count = 0;              // time the key has been held for

static bool shift_mode = false;

// The current morse code
static uint16_t current_code = 0;
static uint8_t current_code_pos = 0;

static bool both_btns_pressed = false;
static bool alt_key_mode = false;

static bool advance_count_active = false;
static uint16_t advance_count = 0;

bool pair_mode = false;


void toggle_led(void)
{
	nrf_gpio_pin_toggle(LED_PIN);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
void sleep_mode_enter(bool all_btns_wake)
{
    ret_code_t err_code;

	// delay as the power key sense isn't debounced
	nrf_delay_ms(30);

	// set which keys will wake the device
	nrf_gpio_cfg_sense_input(POWER_KEY_PIN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
	if (all_btns_wake) {
		nrf_gpio_cfg_sense_input(LEFT_KEY_PIN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
		nrf_gpio_cfg_sense_input(RIGHT_KEY_PIN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
	}

	pair_mode = false;

	nrf_gpio_pin_clear(LED_PIN);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


// ensure that a button press or release is maintained for a minimum time period
static enum key_event debounce_key(key_reading * key, uint32_t pin)
{
	enum key_event event = NO_CHANGE;
	
	key->new = !nrf_gpio_pin_read(pin);

	if ((key->new != key->prev) && (key->new != key->state))
	{
		if (key->new && !key->state)
		{
			event = PRESSED;
		}
		else if (!key->new && key->state)
		{
			event = RELEASED;
		}
		key->state = key->new;
	}

	key->prev = key->new;
	return event;
}


// make the appropriate action for the code that has been entered
void process_code()
{
	uint16_t (*table)[3];
	uint16_t table_size;

	uint16_t code = current_code | (1 << current_code_pos);  // add a 1 bit to mark the end of the code
	
	if (bluetooth_is_connected())
	{
		if (alt_key_mode)
		{
			table = alt_key_table;
			table_size = ALT_KEY_TABLE_SIZE;
		}
		else
		{
			table = key_table;
			table_size = KEY_TABLE_SIZE;
		}

		for (int i = 0; i < table_size; i++)
		{
			if (table[i][0] == code)
			{
				if (table[i][1] == KEY_MOD_LSHIFT)
				{
					shift_mode = !shift_mode;
					NRF_LOG_INFO("SHIFT SET: %d", shift_mode);
				}
				else
				{
					send_key(table[i][1], shift_mode | table[i][2]);
					NRF_LOG_INFO("Send Key: %d / %d", table[i][1], table[i][2]);
					shift_mode = false;
				}
				return;
			}
		}
		NRF_LOG_INFO("Unknown Key Code Entered: %d", code);
	}
}


static void process_key_event(enum key_event event, bool other_state, uint8_t bit)
{
	if (event == PRESSED && !both_btns_pressed)
	{
		extend_inactive_timer();
		key_hold_count = 0;
		if (other_state && (current_code_pos == 0))
		{
			both_btns_pressed = true;
		}
		advance_count_active = false;
	}
	else if (event == RELEASED)
	{
		extend_inactive_timer();
		if (both_btns_pressed)
		{
			if (!other_state)
			{
				NRF_LOG_INFO("ALT KEY MODE");
				both_btns_pressed = false;
				alt_key_mode = true;
			}
		}
		else
		{
			current_code |= bit << current_code_pos;
			current_code_pos++;
		}

		if (key_repeat_mode)
		{
			key_hold_count = 0;
			key_repeat_mode = false;
			current_code = 0;
			current_code_pos = 0;
			NRF_LOG_INFO("CODE RESET");
			alt_key_mode = false;
		}
		else
		{
			advance_count = 0;
			advance_count_active = true;
		}
	}
}


void poll_keys(void)
{
	enum key_event power_event = debounce_key(&power_key, POWER_KEY_PIN);
	if (power_event == RELEASED)
	{
		sleep_mode_enter(false);
	}

	enum key_event left_event = debounce_key(&left_key, LEFT_KEY_PIN);
	process_key_event(left_event, right_key.state, 0);

	enum key_event right_event = debounce_key(&right_key, RIGHT_KEY_PIN);
	process_key_event(right_event, left_key.state, 1);
	
	// key_hold counter
	if (!key_repeat_mode && !both_btns_pressed && (left_key.state || right_key.state))
	{
		key_hold_count++;
		if (key_hold_count > KEY_HOLD_TIME)
		{
			NRF_LOG_INFO("HOLD TRIGGERED");
			advance_count_active = false;
			key_repeat_mode = true;
			key_hold_count = 0;
			if (right_key.state)
			{
				current_code |= 1 << current_code_pos;
			}
			current_code_pos++;
			process_code();
		}
	}

	// key_hold_repeat counter
	if (key_repeat_mode)
	{
		key_hold_count++;
		if (key_hold_count > KEY_REPEAT_TIME)
		{
			process_code();
			key_hold_count = 0;
		}
	}

	// code advance
	if (advance_count_active)
	{
		advance_count++;
		if (advance_count > ADVANCE_TIME)
		{
			process_code();
			current_code = 0;
			current_code_pos = 0;
			NRF_LOG_INFO("CODE RESET");
			alt_key_mode = false;
			advance_count_active = false;
		}
	}
}


void set_device_state(enum device_state state)
{
	switch (state)
	{
	    case DISCONNECTED:
			NRF_LOG_INFO("Event: DISCONNECTED");
			start_led_flash_timer(pair_mode);
			break;
	    case CONNECTED:
			NRF_LOG_INFO("Event: CONNECTED");
			pair_mode = false;
			stop_led_flash_timer();
			nrf_gpio_pin_set(LED_PIN);
			break;
	    case SLEEP:
			NRF_LOG_INFO("Event: SLEEP");
			sleep_mode_enter(true);
			break;
	    case OFF:
			NRF_LOG_INFO("Event: OFF");
			sleep_mode_enter(false);
			break;
	}
}

	
// Function for initializing keys, debouncing and leds
void gpio_init(bool * p_whitelist_active, bool * p_clear_paired)
{
	power_key.prev = false;
	power_key.state = false;
	nrf_gpio_cfg_input(POWER_KEY_PIN, NRF_GPIO_PIN_PULLUP);

	left_key.prev = false;
	left_key.state = false;
	nrf_gpio_cfg_input(LEFT_KEY_PIN, NRF_GPIO_PIN_PULLUP);
	
	right_key.prev = false;
	right_key.state = false;
	nrf_gpio_cfg_input(RIGHT_KEY_PIN, NRF_GPIO_PIN_PULLUP);

	nrf_gpio_cfg_output(LED_PIN);
	nrf_gpio_pin_set(LED_PIN); // set LED on at start to give some indication that the power button worked

	NRF_LOG_INFO("BTN: %d %d %d", !nrf_gpio_pin_read(POWER_KEY_PIN), !nrf_gpio_pin_read(LEFT_KEY_PIN), !nrf_gpio_pin_read(RIGHT_KEY_PIN));

	// check if pair mode
	if(!nrf_gpio_pin_read(LEFT_KEY_PIN) || !nrf_gpio_pin_read(RIGHT_KEY_PIN))
	{
		*p_whitelist_active = false;
		// if both buttons are pressed clear all paired devices
		*p_clear_paired = !nrf_gpio_pin_read(LEFT_KEY_PIN) && !nrf_gpio_pin_read(RIGHT_KEY_PIN);
		pair_mode = true;
		NRF_LOG_INFO("PAIR MODE");

		// wait for the buttons to be released
		while (!nrf_gpio_pin_read(LEFT_KEY_PIN) || !nrf_gpio_pin_read(RIGHT_KEY_PIN));
	}
	else
	{
		*p_whitelist_active = true;
		*p_clear_paired = false;
		pair_mode = false;
	}
}
