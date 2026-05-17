#pragma once

#include <stdint.h>

typedef struct asteroid_t {
    float x, y, x_speed, y_speed;
    int16_t old_x_draw, old_y_draw;
    int8_t radius;
} asteroid_t;

void asteroid_init_random_offscreen(asteroid_t *asteroid);

void asteroid_update_pos(asteroid_t *asteroid, float delta_time);
void asteroid_handle_collision_boundary_temp(asteroid_t *asteroid);

void asteroid_draw_diff(asteroid_t *asteroid);