#include <stdbool.h>
#include <ssd1306.h>
#include <avr/pgmspace.h>

#include "utils.h"

#include "joystick.h"
#include "systime.h"
#include "rgb_led.h"
#include "buzzer.h"
#include "player.h"
#include "asteroid.h"
#include "asteroid_spawner.h"
#include "ui_bar_manager.h"

#define RESET_PIN 8
#define DC_PIN 9
#define CS_PIN 10

ISR(INT0_vect) {
    player.shield_requested = true;
}

int main() {
    joystick_init();
    systime_init();
    rgb_led_init();
    buzzer_init();
    sei();

    buzzer_set_frequency(130);

    ili9341_240x320_spi_init(RESET_PIN, CS_PIN, DC_PIN);
    ili9341_setRotation(3);
    ssd1306_setMode(LCD_MODE_NORMAL);

    ssd1306_setFixedFont(ssd1306xled_font8x16);
    ssd1306_fillScreen16(BACKGROUND_COLOR);

    player_init();
    ui_bar_manager_init();
    asteroid_spawner_init();

    player_draw_init();
    ui_bar_draw_init();

    asteroid_t asteroids[MAX_NO_ASTEROIDS];
    uint8_t no_asteroids = 0;

    uint32_t old_ms_value = systime_get_ms();
    uint32_t buzzer_ms_reference;

    bool game_over = false;

    while (1) {
        if (systime_get_ms() - buzzer_ms_reference > 200) {
            buzzer_set_playing(false);
        }

        if (player.no_lives == 0 && !game_over) {
            ssd1306_setColor(RGB_COLOR16(255, 0, 0));
            ssd1306_printFixed16(120, 100, "GAME OVER!", STYLE_NORMAL);
            game_over = true;
        }

        if (game_over) {
            continue;
        }

        uint32_t ms_value = systime_get_ms();
        float delta_time = (ms_value - old_ms_value) / 1000.0f;
        old_ms_value = ms_value;

        player_update_pos(delta_time);
        player_ckeck_update_invincibility();
        player_check_update_shield();

        player_handle_collision_boudary();
        bool damaged = player_handle_collision_asteroids(asteroids, no_asteroids);

        if (damaged) {
            buzzer_set_playing(true);
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

    return 0;
}