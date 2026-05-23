#include "asteroid.h"

#include "utils.h"
#include <ssd1306.h>

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "rgb_led.h"
#include "systime.h"

#define ASTEROID_COLOR RGB_COLOR16(176, 164, 148)

#define OFFSCREEN_MS 80

static const int16_t circle_heights_lookup_radius_7[] PROGMEM = {
    7, 7, 7, 6, 6, 6, 5, 4, 0
};

static const int16_t circle_heights_lookup_radius_11[] PROGMEM = {
    11, 11, 11, 11, 10, 10, 9, 8, 8, 6, 5, 0
};

static const int16_t circle_heights_lookup_radius_15[] PROGMEM = {
    15, 15, 15, 15, 14, 14, 14, 13, 13, 12, 11, 10, 9, 7, 5, 0
};

bool asteroid_can_be_destroyed(asteroid_t *asteroid) {
    if (asteroid->just_spawned_offscreen) {
        if (systime_get_ms() - asteroid->offscreen_ms_reference > OFFSCREEN_MS) {
            return true;
        }

        return false;
    }

    if (asteroid->x < -asteroid->radius - 1 || asteroid->x > SCREEN_WIDTH + asteroid->radius ||
        asteroid->y < UI_HEIGHT - asteroid->radius - 1 || asteroid->y > SCREEN_HEIGHT + asteroid->radius) {
        return true;
    }

    return false;
}

void asteroid_update_pos(asteroid_t *asteroid, float delta_time) {
    asteroid->x += asteroid->x_speed * delta_time;
    asteroid->y += asteroid->y_speed * delta_time;

    if (-asteroid->radius <= asteroid->x && asteroid->x <= SCREEN_WIDTH + asteroid->radius &&
        UI_HEIGHT - asteroid->radius <= asteroid->y && asteroid->y <= SCREEN_HEIGHT + asteroid->radius) {
        asteroid->just_spawned_offscreen = false;
    }
}

void asteroid_handle_asteroid_collision(asteroid_t *asteroid, asteroid_t *asteroids, uint8_t no_asteroids) {
    for (uint8_t i = 0; i < no_asteroids; i++) {
        asteroid_t* other_asteroid = &asteroids[i];

        if (asteroid == other_asteroid) {
            continue;
        }

        float dx = other_asteroid->x - asteroid->x;
        float dy = other_asteroid->y - asteroid->y;

        float radius_sum = asteroid->radius + other_asteroid->radius;

        if (dx * dx + dy * dy > radius_sum * radius_sum) {
            continue;
        }

        float vx = other_asteroid->x_speed - asteroid->x_speed;
        float vy = other_asteroid->y_speed - asteroid->y_speed;

        float a = vx * vx + vy * vy;
        float b = 2 * (dx * vx + dy * vy);
        float c = dx * dx + dy * dy - radius_sum * radius_sum;

        if (fabs(a) >= FLOAT_TOLERANCE) {
            float t;

            float delta = b * b - 4 * a * c;
            if (delta <= 0) { 
                t = -b / (2 * a);
            } else {
                t = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
            }

            asteroid->x += asteroid->x_speed * t;
            asteroid->y += asteroid->y_speed * t;

            other_asteroid->x += other_asteroid->x_speed * t;
            other_asteroid->y += other_asteroid->y_speed * t;
        }

        dx = other_asteroid->x - asteroid->x;
        dy = other_asteroid->y - asteroid->y;

        float radius2 = (float)asteroid->radius * (float)asteroid->radius;
        float other_radius2 = (float)other_asteroid->radius * (float)other_asteroid->radius;

        float distance2 = dx * dx + dy * dy;
        if (distance2 <= FLOAT_TOLERANCE) {
            distance2 = 1;
        }

        float constant = 2.0f / ((radius2 + other_radius2) * distance2);
        float dot_product = vx * dx + vy * dy;

        asteroid->x_speed -= constant * other_radius2 * dot_product * (-dx);
        asteroid->y_speed -= constant * other_radius2 * dot_product * (-dy);

        other_asteroid->x_speed -= constant * radius2 * dot_product * dx;
        other_asteroid->y_speed -= constant * radius2 * dot_product * dy;
    }
}

void asteroid_draw_diff(asteroid_t *asteroid) {
    int16_t x_draw = round(asteroid->x);
	int16_t y_draw = round(asteroid->y);

    if (x_draw == asteroid->old_x_draw && y_draw == asteroid->old_y_draw) {
        return;
	}

    const int16_t *circle_heights_lookup_table = NULL;

    if (asteroid->radius == 7) {
        circle_heights_lookup_table = circle_heights_lookup_radius_7;
    } else if (asteroid->radius == 11) {
        circle_heights_lookup_table = circle_heights_lookup_radius_11;
    } else {
        circle_heights_lookup_table = circle_heights_lookup_radius_15;
    }

    ssd1306_setColor(BACKGROUND_COLOR);

    for (int16_t old_dx = -asteroid->radius; old_dx <= asteroid->radius; old_dx++) {
        int16_t old_x = asteroid->old_x_draw + old_dx;

        if (old_x < 0 || old_x >= SCREEN_WIDTH) {
            continue;
        }

        int16_t old_height = pgm_read_word(&circle_heights_lookup_table[abs(old_dx)]);
        int16_t height = -1;

        int16_t dx = x_draw - old_x;
        if (abs(dx) <= asteroid->radius) {
            height = pgm_read_word(&circle_heights_lookup_table[abs(dx)]);
        }

        if (height == -1) {
            int16_t y_up = asteroid->old_y_draw - old_height;
            if (y_up >= SCREEN_HEIGHT) {
                continue;
            } else if (y_up < UI_HEIGHT) {
                y_up = UI_HEIGHT;
            }

            int16_t y_down = asteroid->old_y_draw + old_height;
            if (y_down < UI_HEIGHT) {
                continue;
            } else if (y_down >= SCREEN_HEIGHT) {
                y_down = SCREEN_HEIGHT - 1;
            }

            ssd1306_drawVLine16(old_x, y_up, y_down);
            continue;
        }

        int16_t y_draw_diff = y_draw - asteroid->old_y_draw;

        int16_t top_distance = (y_draw_diff - height) - (-old_height);
        int16_t bottom_distance = old_height - (y_draw_diff + height);

        if (top_distance > 0) {
            int16_t y_up = asteroid->old_y_draw - old_height;
            if (y_up >= SCREEN_HEIGHT) {
                continue;
            } else if (y_up < UI_HEIGHT) {
                y_up = UI_HEIGHT;
            }

            int16_t y_down = y_up + top_distance - 1;
            if (y_down < UI_HEIGHT) {
                continue;
            } else if (y_down >= SCREEN_HEIGHT) {
                y_down = SCREEN_HEIGHT - 1;
            }

            ssd1306_drawVLine16(old_x, y_up, y_down);
        }

        if (bottom_distance > 0) {
            int16_t y_up = y_draw + height + 1;
            if (y_up >= SCREEN_HEIGHT) {
                continue;
            } else if (y_up < UI_HEIGHT) {
                y_up = UI_HEIGHT;
            }

            int16_t y_down = y_up + bottom_distance - 1;
            if (y_down < UI_HEIGHT) {
                continue;
            } else if (y_down >= SCREEN_HEIGHT) {
                y_down = SCREEN_HEIGHT - 1;
            }
            
            ssd1306_drawVLine16(old_x, y_up, y_down);
        }
    }

    ssd1306_setColor(ASTEROID_COLOR);

    for (int16_t dx = 0; dx <= asteroid->radius; dx++) {
        int16_t height = pgm_read_word(&circle_heights_lookup_table[dx]);

        int16_t y_up = y_draw - height;
        if (y_up >= SCREEN_HEIGHT) {
            continue;
        } else if (y_up < UI_HEIGHT) {
            y_up = UI_HEIGHT;
        }

        int16_t y_down = y_draw + height;
        if (y_down < UI_HEIGHT) {
            continue;
        } else if (y_down >= SCREEN_HEIGHT) {
            y_down = SCREEN_HEIGHT - 1;
        }

        int16_t x_left = x_draw - dx;

        if (0 <= x_left && x_left < SCREEN_WIDTH) {
            ssd1306_drawVLine16(x_left, y_up, y_down);
        }

        if (dx == 0) {
            continue;
        }

        int16_t x_right = x_draw + dx;

        if (0 <= x_right && x_right < SCREEN_WIDTH) {
            ssd1306_drawVLine16(x_right, y_up, y_down);
        }
    }

    asteroid->old_x_draw = x_draw;
	asteroid->old_y_draw = y_draw;
}