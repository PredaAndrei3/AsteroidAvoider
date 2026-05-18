#pragma once

#include <stdint.h>
#include "asteroid.h"

#define MAX_NO_ASTEROIDS 6

typedef struct asteroid_spawner_t {
    uint32_t average_waiting_time_ms;
    uint32_t deviation_waiting_time_ms;

    uint8_t min_speed;
    uint8_t max_speed;

    uint8_t radius7_weight;
    uint8_t radius11_weight;
    uint8_t radius15_weight;

    uint32_t waiting_time_ms;
    uint32_t waiting_time_ms_reference;

    uint32_t update_ms_reference;
    uint8_t no_updates;
} asteroid_spawner_t;

extern asteroid_spawner_t asteroid_spawner;

void asteroid_spawner_init();
void asteroid_spawner_update(asteroid_t *asteroids, uint8_t *no_asteroids);