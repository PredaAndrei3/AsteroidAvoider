#pragma once

#include <stdint.h>
#include <stdbool.h>

void buzzer_init();
void buzzer_set_frequency(uint16_t frequency);
void buzzer_set_playing(bool play);

