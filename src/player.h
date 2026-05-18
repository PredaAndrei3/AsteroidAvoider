#pragma once

#include <stdint.h>
#include "asteroid.h"

typedef struct player_t {
    float x, y, x_speed_inertia, y_speed_inertia;
    int16_t old_x_draw, old_y_draw;
    uint8_t no_lives;
    volatile uint8_t no_shields;
    bool invincible, shield, old_shield, display;
    volatile bool shield_requested;
    uint32_t invincible_ms_reference;
    volatile uint32_t shield_ms_reference;
} player_t;

extern player_t player;

void player_init();

void player_update_pos(float delta_time);
void player_ckeck_update_invincibility();
void player_check_update_shield();

void player_handle_collision_boundary();
bool player_handle_collision_asteroids(asteroid_t *asteroids, uint8_t no_asteroids);

void player_draw_init();
void player_draw_diff();
void player_draw_invincible();