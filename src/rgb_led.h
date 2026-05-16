#pragma once

typedef enum rgb_led_state_t {
    GREEN,
    YELLOW,
    BLINKING_RED,
    OFF
} rgb_led_state_t;

void rgb_led_init();
void rgb_led_set_state(rgb_led_state_t state);