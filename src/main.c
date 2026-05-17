#include <stdbool.h>
#include <ssd1306.h>
#include <avr/pgmspace.h>

#include "defines.h"

#include "joystick.h"
#include "systime.h"
#include "rgb_led.h"
#include "buzzer.h"
#include "player.h"

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

    ssd1306_fillScreen16(BACKGROUND_COLOR);

    ssd1306_setColor(RGB_COLOR16(0, 0, 0));
    ssd1306_fillRect16(0, 0, SCREEN_WIDTH - 1, 23);

    player_draw_init();

    uint32_t old_ms_value = 0;

    while (1) {
        uint32_t ms_value = systime_get_ms();
        float delta_time = (ms_value - old_ms_value) / 1000.0f;
        old_ms_value = ms_value;

        player_update_pos_joystick(delta_time);
        player_handle_collision_boudary();

        player_draw_diff();
    }

    return 0;
}