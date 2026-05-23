#pragma once

#include <stdbool.h>

void joystick_init();

float joystick_get_x();
float joystick_get_y();

void joystick_update_sw_pressed_status();
bool joystick_is_sw_pressed();

void joystick_disable_sw();
void joystick_enable_sw();