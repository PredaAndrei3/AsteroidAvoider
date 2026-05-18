#pragma once

#include <stdint.h>

#include "player.h"

#define LIVES_LENGTH 8

typedef struct ui_bar_manager_t {
    char lives_text[LIVES_LENGTH + 1];
    int16_t old_x_current_shield, x_current_shield;
    bool lives_changed, shields_changed;
} ui_bar_manager_t;

extern ui_bar_manager_t ui_bar_manager;

void ui_bar_manager_init();

void ui_bar_manager_update();

void ui_bar_draw_init();
void ui_bar_draw_diff();