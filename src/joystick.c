#include "joystick.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "systime.h"

#define SW_DDR DDRD
#define SW_PORT PORTD
#define SW PD2

#define MIDDLE_VAL 512
#define TOLERANCE 50

#define ADC_FACTOR 0.2f

volatile bool sw_requested;
bool sw_pressed;

ISR(INT0_vect) {
    sw_requested = true;
}

void joystick_init() {
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    SW_DDR &= ~(1 << SW);
    SW_PORT |= (1 << SW);

    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
}

static float get_value() {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    int16_t raw_value = ADC;
    if (MIDDLE_VAL - TOLERANCE <= raw_value && raw_value <= MIDDLE_VAL + TOLERANCE) {
        raw_value = MIDDLE_VAL;
    }

    if (raw_value > MIDDLE_VAL) {
        raw_value -= TOLERANCE;
    } else if (raw_value < MIDDLE_VAL) {
        raw_value += TOLERANCE;
    }

    return (raw_value - MIDDLE_VAL) * ADC_FACTOR;
}

float joystick_get_x() {
    ADMUX &= ~(0b00011111);

    return get_value();
}

float joystick_get_y() {
    ADMUX &= ~(0b00011111);
    ADMUX |= (1 << MUX0);

    return get_value();
}

void joystick_update_sw_pressed_status() {
    EIMSK &= ~(1 << INT0);

    sw_pressed = sw_requested;
    sw_requested = false;

    EIMSK |= (1 << INT0);
}

bool joystick_is_sw_pressed() {
    return sw_pressed;
}

void joystick_disable_sw() {
    EIMSK &= ~(1 << INT0);
    EIFR |= (1 << INTF0);

    sw_requested = sw_pressed = false;
}

void joystick_enable_sw() {
    EIFR |= (1 << INTF0);
    EIMSK |= (1 << INT0);
}
