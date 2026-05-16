#include "joystick.h"

#include <avr/io.h>

#define SW PD2

#define MIDDLE_VAL 512
#define TOLERANCE 50

void joystick_init() {
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

float joystick_get_x() {
    ADMUX &= ~(0x0F);
    ADMUX |= (1 << MUX0);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    int16_t raw_value = ADC;
    if (MIDDLE_VAL - TOLERANCE <= raw_value && raw_value <= MIDDLE_VAL + TOLERANCE) {
        raw_value = MIDDLE_VAL;
    }

    return (raw_value - MIDDLE_VAL) / 5.0f;
}

float joystick_get_y() {
    ADMUX &= ~(0x0F);
    ADMUX |= (1 << MUX1);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    int16_t raw_value = ADC;
    if (MIDDLE_VAL - TOLERANCE <= raw_value && raw_value <= MIDDLE_VAL + TOLERANCE) {
        raw_value = MIDDLE_VAL;
    }

    return (raw_value - MIDDLE_VAL) / 5.0f;
}
