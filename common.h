#ifndef COMMON_H
#define COMMON_H

#include "nrf_log.h"

// Keyboard

enum device_state { DISCONNECTED, CONNECTED, SLEEP, OFF };

void set_device_state(enum device_state state);

void toggle_led(void);

void advance_code_pos(void);

void poll_keys(void);

void gpio_init(bool * p_whitelist_active, bool * p_clear_paired);


// Bluetooth

void bluetooth_init();

void advertising_start(bool p_whitelist_active, bool p_clear_paired);

bool bluetooth_is_connected(void);

void send_key(uint8_t key, bool shift);

void battery_level_update(uint8_t level);

void delete_bonds(void);


// Timers

void timers_init(void);

void timers_start(void);

void extend_inactive_timer();

void start_led_flash_timer(bool fast);

void stop_led_flash_timer(void);


// Battery

void battery_init(void);

uint8_t get_battery_level(void);

#endif //COMMON_H
