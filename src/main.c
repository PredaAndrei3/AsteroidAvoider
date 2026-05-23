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

static void random_seed_init() {
    uint8_t old_admux = ADMUX;
    uint8_t old_adcsra = ADCSRA;

    ADMUX = (1 << REFS0) | (1 << MUX2) | (1 << MUX0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    srand(ADC);

    ADMUX = old_admux;
    ADCSRA = old_adcsra;
}

int main() {
    joystick_init();
    systime_init();
    rgb_led_init();
    buzzer_init();
    random_seed_init();

    ili9341_240x320_spi_init(RESET_PIN, CS_PIN, DC_PIN);
    ili9341_setRotation(3);
    ssd1306_setMode(LCD_MODE_NORMAL);
    ssd1306_setFixedFont(ssd1306xled_font8x16);

    sei();

    game_run();

    return 0;
}