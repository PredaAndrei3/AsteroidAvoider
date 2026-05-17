#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct asteroid_t {
    float x, y, x_speed, y_speed;
    int16_t old_x_draw, old_y_draw;
    int8_t radius;

    bool just_spawned_offscreen;
} asteroid_t;

bool asteroid_can_be_destroyed(asteroid_t *asteroid);

void asteroid_update_pos(asteroid_t *asteroid, float delta_time);

void asteroid_draw_diff(asteroid_t *asteroid);