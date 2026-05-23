#include "game.h"

#include <ssd1306.h>
#include <stdio.h>

#include "utils.h"

#include "player.h"
#include "asteroid.h"
#include "asteroid_spawner.h"
#include "ui_bar_manager.h"

#include "systime.h"
#include "joystick.h"
#include "buzzer.h"

uint32_t EEMEM eeprom_best_time_survived;

asteroid_t asteroids[MAX_NO_ASTEROIDS];
uint8_t no_asteroids = 0;

uint32_t old_ms_value;
uint32_t buzzer_ms_reference;
uint32_t time_survived_ms_reference;

bool game_over = false;
bool buzzer_playing = false;

uint8_t game_over_music_index = 0;

#define HIT_MS 200

#define GAME_OVER_NO_FREQUENCIES 3

uint16_t game_over_music_frequencies[] = {220, 164, 155};
uint16_t game_over_music_durations[] = {250, 250, 600};

#define GAME_OVER_TEXT_COLOR RGB_COLOR16(255, 0, 0)

#define GAME_OVER_TEXT_X 120
#define GAME_OVER_TEXT_Y 60

#define TIME_TEXT_X 120
#define TIME_TEXT_Y 90

#define BEST_TIME_TEXT_X 80
#define BEST_TIME_TEXT_Y 120

#define NEW_RECORD_TEXT_X 116
#define NEW_RECORD_TEXT_Y 150

#define PLAY_AGAIN_TEXT_X 45
#define PLAY_AGAIN_TEXT_Y 200

static void game_reset() {
    buzzer_set_playing(false);
    ssd1306_fillScreen16(BACKGROUND_COLOR);

    player_init();
    ui_bar_manager_init();
    asteroid_spawner_init();

    player_draw_init();
    ui_bar_draw_init();

    no_asteroids = 0;

    old_ms_value = systime_get_ms();
    time_survived_ms_reference = systime_get_ms();

    game_over = false;
    buzzer_playing = false;

    game_over_music_index = 0;
}

static void draw_game_over_screen() {
    float time_survived = (systime_get_ms() - time_survived_ms_reference) / 1000.0f;

    ssd1306_setColor(GAME_OVER_TEXT_COLOR);
    ssd1306_printFixed16(GAME_OVER_TEXT_X, GAME_OVER_TEXT_Y, "GAME OVER!", STYLE_NORMAL);

    ssd1306_setColor(TEXT_COLOR);

    char time_text[6 + 10 + 1];
    memcpy(time_text, "Time: ", 6);
    sprintf(time_text + 6, "%lu", (uint32_t)floor(time_survived));

    ssd1306_printFixed16(TIME_TEXT_X, TIME_TEXT_Y, time_text, STYLE_NORMAL);

    uint32_t best_time_survived = eeprom_read_dword(&eeprom_best_time_survived);
    if (best_time_survived == UINT32_MAX) {
        eeprom_write_dword(&eeprom_best_time_survived, 0);
        best_time_survived = 0;
    }

    bool new_record = false;

    if (time_survived > best_time_survived) {
        best_time_survived = time_survived;
        eeprom_write_dword(&eeprom_best_time_survived, time_survived);

        new_record = true;
    }

    char best_time_text[11 + 10 + 1];
    memcpy(best_time_text, "Best time: ", 11);
    sprintf(best_time_text + 11, "%lu", (uint32_t)floor(best_time_survived));

    ssd1306_printFixed16(BEST_TIME_TEXT_X, BEST_TIME_TEXT_Y, best_time_text, STYLE_NORMAL);

    if (new_record) {
        ssd1306_printFixed16(NEW_RECORD_TEXT_X, NEW_RECORD_TEXT_Y, "NEW RECORD!", STYLE_NORMAL);
    }

    ssd1306_printFixed16(PLAY_AGAIN_TEXT_X, PLAY_AGAIN_TEXT_Y, "Press joystick to play again!", STYLE_NORMAL);
}

void game_run() {
    game_reset();

    while (true) {
        joystick_update_sw_pressed_status();

        if (buzzer_playing) {
            if (!game_over) {
                if (systime_get_ms() - buzzer_ms_reference > HIT_MS) {
                    buzzer_set_playing(false);
                    buzzer_playing = false;
                }
            } else {
                if (systime_get_ms() - buzzer_ms_reference > game_over_music_durations[game_over_music_index]) {
                    game_over_music_index++;

                    if (game_over_music_index < GAME_OVER_NO_FREQUENCIES) {
                        buzzer_set_frequency(game_over_music_frequencies[game_over_music_index]);
                        buzzer_ms_reference = systime_get_ms();
                    } else {
                        buzzer_set_playing(false);
                        buzzer_playing = false;
                    }
                }
            }
        }

        if (game_over) {
            if (joystick_is_sw_pressed()) {
                joystick_disable_sw();
                game_reset();
                joystick_enable_sw();
            }

            continue;
        }

        if (player.no_lives == 0 && !game_over) {
            game_over = true;
            draw_game_over_screen();

            buzzer_set_frequency(game_over_music_frequencies[game_over_music_index]);

            buzzer_set_playing(true);
            buzzer_playing = true;

            buzzer_ms_reference = systime_get_ms();
            continue;
        }

        uint32_t ms_value = systime_get_ms();
        float delta_time = (ms_value - old_ms_value) / 1000.0f;
        old_ms_value = ms_value;

        player_update_pos(delta_time);
        player_ckeck_update_invincibility();
        player_check_update_shield();

        player_handle_collision_boundary();
        bool damaged = player_handle_collision_asteroids(asteroids, no_asteroids);

        if (damaged && player.no_lives != 0) {
            buzzer_set_frequency(130);

            buzzer_set_playing(true);
            buzzer_playing = true;

            buzzer_ms_reference = systime_get_ms();
        }

        asteroid_spawner_update(asteroids, &no_asteroids);

        for (uint8_t i = 0; i < no_asteroids; i++) {
            asteroid_update_pos(&asteroids[i], delta_time);
            asteroid_handle_asteroid_collision(&asteroids[i], asteroids, no_asteroids);
        }

        if (player.invincible) {
            player_draw_invincible();
        } else {
            player_draw_diff();
        }

        for (uint8_t i = 0; i < no_asteroids; i++) {
            asteroid_draw_diff(&asteroids[i]);

            if (asteroid_can_be_destroyed(&asteroids[i])) {
                memcpy(&asteroids[i], &asteroids[no_asteroids - 1], sizeof(asteroid_t));
                no_asteroids--;

                i--;
                continue;
            }
        }

        ui_bar_manager_update();
        ui_bar_draw_diff();
    }
}