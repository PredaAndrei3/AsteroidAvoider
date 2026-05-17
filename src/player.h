#pragma once

#include <stdint.h>

typedef struct player_t {
    float x, y;
    int16_t old_x_draw, old_y_draw;
    uint8_t no_lives;
} player_t;

extern player_t player;

void player_init();

void player_update_pos_joystick(float delta_time);
void player_handle_collision_boudary();

void player_draw_init();
void player_draw_diff();