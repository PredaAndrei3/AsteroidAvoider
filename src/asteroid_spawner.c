#include "asteroid_spawner.h"

#include "utils.h"
#include "systime.h"
#include <stdbool.h>
#include <math.h>

#define TOLERANCE 15
#define UPDATE_MS 10000

asteroid_spawner_t asteroid_spawner;

void asteroid_spawner_init() {
    asteroid_spawner.average_waiting_time_ms = 1500;
    asteroid_spawner.deviation_waiting_time_ms = 500;
    
    asteroid_spawner.min_speed = 70;
    asteroid_spawner.max_speed = 100;
    
    asteroid_spawner.radius7_weight = 100;
    asteroid_spawner.radius11_weight = 70;
    asteroid_spawner.radius15_weight = 30;

    int a = asteroid_spawner.average_waiting_time_ms - asteroid_spawner.deviation_waiting_time_ms;
    int b = asteroid_spawner.average_waiting_time_ms + asteroid_spawner.deviation_waiting_time_ms;

    asteroid_spawner.waiting_time_ms = random_range(a, b);
    asteroid_spawner.waiting_time_ms_reference = systime_get_ms();

    asteroid_spawner.update_ms_reference = systime_get_ms();
    asteroid_spawner.no_updates = 20;
}

void asteroid_spawner_update(asteroid_t *asteroids, uint8_t *no_asteroids) {
    if (asteroid_spawner.no_updates > 0 && systime_get_ms() - asteroid_spawner.update_ms_reference > UPDATE_MS) {
        asteroid_spawner.average_waiting_time_ms -= 40;
        asteroid_spawner.deviation_waiting_time_ms -= 10;

        asteroid_spawner.min_speed += 2;
        asteroid_spawner.max_speed += 5;

        asteroid_spawner.radius7_weight -= 3;
        asteroid_spawner.radius15_weight--;

        asteroid_spawner.no_updates--;
        asteroid_spawner.update_ms_reference = systime_get_ms();
    }

    if (systime_get_ms() - asteroid_spawner.waiting_time_ms_reference < asteroid_spawner.waiting_time_ms) {
        return;
    }

    if (*no_asteroids >= MAX_NO_ASTEROIDS) {
        return;
    }

    asteroid_t *asteroid = &asteroids[*no_asteroids];

    int weight_sum = asteroid_spawner.radius7_weight + asteroid_spawner.radius11_weight + asteroid_spawner.radius15_weight;
    int radius_val = random_range(1, weight_sum);

    if (radius_val <= asteroid_spawner.radius7_weight) {
        asteroid->radius = 7;
    } else if (radius_val <= asteroid_spawner.radius7_weight + asteroid_spawner.radius11_weight) {
        asteroid->radius = 11;
    } else {
        asteroid->radius = 15;
    }

    bool spawn_from_sides = random_range(0, 1);

    if (spawn_from_sides) {
        asteroid->y = random_range(UI_HEIGHT - asteroid->radius - 1, SCREEN_HEIGHT - 1 + asteroid->radius + 1);

        bool left = random_range(0, 1);
        if (left) {
            asteroid->x = -asteroid->radius - 1;
        } else {
            asteroid->x = SCREEN_WIDTH - 1 + asteroid->radius + 1;
        }
    } else {
        asteroid->x = random_range(-asteroid->radius - 1, SCREEN_WIDTH - 1 + asteroid->radius + 1);

        bool up = random_range(0, 1);
        if (up) {
            asteroid->y = UI_HEIGHT - asteroid->radius - 1;
        } else {
            asteroid->y = SCREEN_HEIGHT - 1 + asteroid->radius + 1;
        }
    }

    asteroid->old_x_draw = round(asteroid->x);
	asteroid->old_y_draw = round(asteroid->y);

    asteroid->just_spawned_offscreen = true;

    float speed = random_range(asteroid_spawner.min_speed, asteroid_spawner.max_speed);

    int16_t padding = 2 * asteroid->radius + 1 + TOLERANCE;

    int16_t x = random_range(padding, SCREEN_WIDTH - 1 - padding);
    int16_t y = random_range(padding, SCREEN_HEIGHT - 1 - padding);

    float distance = hypot(x - asteroid->x, y - asteroid->y);

    asteroid->x_speed = speed * (x - asteroid->x) / distance;
    asteroid->y_speed = speed * (y - asteroid->y) / distance;

    (*no_asteroids)++;

    int a = asteroid_spawner.average_waiting_time_ms - asteroid_spawner.deviation_waiting_time_ms;
    int b = asteroid_spawner.average_waiting_time_ms + asteroid_spawner.deviation_waiting_time_ms;

    asteroid_spawner.waiting_time_ms = random_range(a, b);
    asteroid_spawner.waiting_time_ms_reference = systime_get_ms();
}
