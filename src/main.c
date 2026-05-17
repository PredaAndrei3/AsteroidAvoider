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

#define RESET_PIN 8
#define DC_PIN 9
#define CS_PIN 10

int main() {
    joystick_init();
    systime_init();
    rgb_led_init();
    buzzer_init();
    sei();

    ili9341_240x320_spi_init(RESET_PIN, CS_PIN, DC_PIN);
    ili9341_setRotation(3);
    ssd1306_setMode(LCD_MODE_NORMAL);

    player_init();

    srand(100);

    asteroid_t asteroids[5];

    for (uint8_t i = 0; i < 5; i++) {
        asteroid_init_random_offscreen(&asteroids[i]);
    }

    ssd1306_fillScreen16(BACKGROUND_COLOR);

    ssd1306_setColor(RGB_COLOR16(0, 0, 0));
    ssd1306_fillRect16(0, 0, SCREEN_WIDTH - 1, 23);

    player_draw_init();

    uint32_t old_ms_value = systime_get_ms();

    while (1) {
        uint32_t ms_value = systime_get_ms();
        float delta_time = (ms_value - old_ms_value) / 1000.0f;
        old_ms_value = ms_value;

        player_update_pos_joystick(delta_time);
        player_handle_collision_boudary();

        for (uint8_t i = 0; i < 5; i++) {
            asteroid_update_pos(&asteroids[i], delta_time);
            asteroid_handle_collision_boundary_temp(&asteroids[i]);
        }

        player_draw_diff();

        for (uint8_t i = 0; i < 5; i++) {
            asteroid_draw_diff(&asteroids[i]);
        }
    }

    return 0;
}