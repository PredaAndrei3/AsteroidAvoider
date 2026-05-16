#include "systime.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t systime_ms = 0;

ISR(TIMER0_COMPA_vect) {
    systime_ms++;
}

void systime_init() {
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << CS01) | (1 << CS00);
    TIMSK0 |= (1 << OCIE0A);
    OCR0A = 249;
}

uint32_t systime_get_ms() {
    TIMSK0 &= ~(1 << OCIE0A);
    uint32_t val = systime_ms;
    TIMSK0 |= (1 << OCIE0A);

    return val;
}
