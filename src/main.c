#include <ssd1306.h>

#include "joystick.h"
#include "systime.h"
#include "rgb_led.h"
#include "buzzer.h"

#include "game.h"
#include "utils.h"

#define RESET_PIN 8
#define DC_PIN 9
#define CS_PIN 10

int main() {
    joystick_init();
    systime_init();
    rgb_led_init();
    buzzer_init();

    ili9341_240x320_spi_init(RESET_PIN, CS_PIN, DC_PIN);
    ili9341_setRotation(3);
    ssd1306_setMode(LCD_MODE_NORMAL);
    ssd1306_setFixedFont(ssd1306xled_font8x16);

    sei();

    game_run();

    return 0;
}