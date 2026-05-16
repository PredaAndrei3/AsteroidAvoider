#include "rgb_led.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_DDR DDRD
#define LED_PORT PORTD
#define R PD5
#define G PD6

ISR(TIMER1_COMPA_vect) {
    LED_PORT ^= (1 << R);
}

void rgb_led_init() {
    LED_DDR |= (1 << R) | (1 << G);

    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 15624;
}

void rgb_led_set_state(rgb_led_state_t state) {
    TCCR1B &= ~(1 << CS12);
    TCNT1 = 0;
    TIFR1 |= (1 << OCF1A);

    switch (state) {
    case GREEN:
        LED_PORT &= ~(1 << R);
        LED_PORT |= (1 << G);
        break;
    case YELLOW:
        LED_PORT |= (1 << R);
        LED_PORT |= (1 << G);
        break;
    case BLINKING_RED:
        LED_PORT |= (1 << R);
        LED_PORT &= ~(1 << G);

        TCCR1B |= (1 << CS12);
        break;
    default: //EMPTY
        LED_PORT &= ~(1 << R);
        LED_PORT &= ~(1 << G);
        break;
    }
}
