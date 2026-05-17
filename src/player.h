#pragma once

#include <stdint.h>
#include "asteroid.h"

#define INVINCIBLE_MS 1000

typedef struct player_t {
    float x, y, x_speed_inertia, y_speed_inertia;
    int16_t old_x_draw, old_y_draw;
    uint8_t no_lives;
    bool invincible, shield, display;
    uint32_t invincible_ms_reference, shield_ms_reference;
} player_t;

extern player_t player;

void player_init();

void player_update_pos(float delta_time);
void player_ckeck_update_invincibility();

void player_handle_collision_boudary();
bool player_handle_collision_asteroids(asteroid_t *asteroids, uint8_t no_asteroids);

void player_draw_init();
void player_draw_diff();
void player_draw_invincible();