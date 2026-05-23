#include "ui_bar_manager.h"

#include "utils.h"

#include <ssd1306.h>
#include <string.h>

#define UI_BAR_COLOR RGB_COLOR16(0, 0, 0)

#define NUMBER_INDEX 7

#define LIVES_TEXT_X 4
#define SHIELDS_TEXT_X 190

#define TEXT_Y 3

#define SHIELD_UI_PADDING 4
#define SHIELD_UI_SIZE (UI_HEIGHT - 2 * SHIELD_UI_PADDING)

ui_bar_manager_t ui_bar_manager;

void ui_bar_manager_init() {
    strcpy(ui_bar_manager.lives_text, "Lives:  ");
    ui_bar_manager.lives_text[NUMBER_INDEX] = player.no_lives + '0';

    ui_bar_manager.x_current_shield = SCREEN_WIDTH - player.no_shields * (SHIELD_UI_PADDING + SHIELD_UI_SIZE);
    ui_bar_manager.old_x_current_shield = ui_bar_manager.x_current_shield;

    ui_bar_manager.lives_changed = ui_bar_manager.shields_changed = false;
}

void ui_bar_manager_update() {
    if (player.no_lives != ui_bar_manager.lives_text[7] - '0') {
        ui_bar_manager.lives_text[7] = player.no_lives + '0';
        ui_bar_manager.lives_changed = true;
    }

    ui_bar_manager.old_x_current_shield = ui_bar_manager.x_current_shield;
    ui_bar_manager.x_current_shield = SCREEN_WIDTH - player.no_shields * (SHIELD_UI_PADDING + SHIELD_UI_SIZE);

    if (ui_bar_manager.x_current_shield != ui_bar_manager.old_x_current_shield) {
        ui_bar_manager.shields_changed = true;
    }
}

void ui_bar_draw_init() {
    ssd1306_setColor(UI_BAR_COLOR);
    ssd1306_fillRect16(0, 0, SCREEN_WIDTH - 1, UI_HEIGHT - 1);

    ssd1306_setColor(TEXT_COLOR);
    ssd1306_printFixed16(LIVES_TEXT_X, TEXT_Y, ui_bar_manager.lives_text, STYLE_NORMAL);

    ssd1306_setColor(SHIELD_COLOR);

    uint16_t x_shield = SCREEN_WIDTH - SHIELD_UI_PADDING - SHIELD_UI_SIZE;

    while (true) {
        ssd1306_drawRect16(
            x_shield,
            SHIELD_UI_PADDING,
            x_shield + SHIELD_UI_SIZE - 1,
            SHIELD_UI_PADDING + SHIELD_UI_SIZE - 1
        );

        if (x_shield == ui_bar_manager.x_current_shield) {
            break;
        }

        x_shield -= SHIELD_UI_PADDING + SHIELD_UI_SIZE;
    }

    ssd1306_setColor(TEXT_COLOR);
    ssd1306_printFixed16(SHIELDS_TEXT_X, TEXT_Y, "Shields:", STYLE_NORMAL);
}

void ui_bar_draw_diff() {
    if (ui_bar_manager.lives_changed) {
        ui_bar_manager.lives_changed = false;

        ssd1306_setColor(TEXT_COLOR);
        ssd1306_printFixed16(LIVES_TEXT_X, TEXT_Y, ui_bar_manager.lives_text, STYLE_NORMAL);
    }

    if (ui_bar_manager.shields_changed) {
        ui_bar_manager.shields_changed = false;

        ssd1306_setColor(UI_BAR_COLOR);

        uint16_t x_shield = ui_bar_manager.old_x_current_shield;

        while (x_shield != ui_bar_manager.x_current_shield) {
            ssd1306_drawRect16(
                x_shield,
                SHIELD_UI_PADDING,
                x_shield + SHIELD_UI_SIZE - 1,
                SHIELD_UI_PADDING + SHIELD_UI_SIZE - 1
            );

            x_shield += SHIELD_UI_PADDING + SHIELD_UI_SIZE;
        }
    }
}
