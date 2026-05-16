#include "buzzer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define BUZZER_DDR DDRD
#define BUZZER_PORT PORTD
#define BUZZER PD7

void buzzer_init() {
    BUZZER_DDR |= (1 << BUZZER);

    TCCR2A |= (1 << WGM21);
    TIMSK2 |= (1 << OCIE2A);
}

void buzzer_set_frequency(uint16_t frequency) {
    OCR2A = F_CPU / (2 * 256 * (uint32_t)frequency) - 1;
}

void buzzer_set_playing(bool play) {
    if (play) {
        TCCR2B |= (1 << CS22) | (1 << CS21);
    } else {
        TCCR2B &= ~((1 << CS22) | (1 << CS21));
        TCNT2 = 0;
        TIFR2 |= (1 << OCF2A);

        BUZZER_PORT &= ~(1 << BUZZER);
    }
}

ISR(TIMER2_COMPA_vect) {
    BUZZER_PORT ^= (1 << BUZZER);
}