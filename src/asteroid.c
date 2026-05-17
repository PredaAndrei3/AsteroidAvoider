#include "asteroid.h"

#include "defines.h"
#include <ssd1306.h>

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "rgb_led.h"

#define ASTEROID_COLOR RGB_COLOR16(176, 164, 148)

#define MIN_SPEED 20
#define MAX_SPEED 100

#define TOLERANCE 15

static const int16_t circle_heights_lookup_radius_7[] PROGMEM = {
    7, 7, 7, 6, 6, 6, 5, 4, 0
};

static const int16_t circle_heights_lookup_radius_11[] PROGMEM = {
    11, 11, 11, 11, 10, 10, 9, 8, 8, 6, 5, 0
};

static const int16_t circle_heights_lookup_radius_15[] PROGMEM = {
    15, 15, 15, 15, 14, 14, 14, 13, 13, 12, 11, 10, 9, 7, 5, 0
};

static int random_range(int a, int b) {
    return rand() % (b - a + 1) + a;
}

void asteroid_init_random_offscreen(asteroid_t *asteroid) {
    asteroid->radius = 7 + 4 * random_range(0, 2);

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

    float speed = random_range(MIN_SPEED, MAX_SPEED);

    int16_t padding = 2 * asteroid->radius + 1 + TOLERANCE;

    int16_t x = random_range(padding, SCREEN_WIDTH - 1 - padding);
    int16_t y = random_range(padding, SCREEN_HEIGHT - 1 - padding);

    float distance = hypot(x - asteroid->x, y - asteroid->y);

    asteroid->x_speed = speed * (x - asteroid->x) / distance;
    asteroid->y_speed = speed * (y - asteroid->y) / distance;
}

void asteroid_update_pos(asteroid_t *asteroid, float delta_time) {
    asteroid->x += asteroid->x_speed * delta_time;
    asteroid->y += asteroid->y_speed * delta_time;
}

void asteroid_handle_collision_boundary_temp(asteroid_t *asteroid) {
    if (asteroid->x < asteroid->radius) {
        asteroid->x = asteroid->radius;
        asteroid->x_speed = -asteroid->x_speed;
    } else if (asteroid->x > SCREEN_WIDTH - 1 - asteroid->radius) {
        asteroid->x = SCREEN_WIDTH - 1 - asteroid->radius;
        asteroid->x_speed = -asteroid->x_speed;
    }

    if (asteroid->y < UI_HEIGHT + asteroid->radius) {
        asteroid->y = UI_HEIGHT + asteroid->radius;
        asteroid->y_speed = -asteroid->y_speed;
    } else if (asteroid->y > SCREEN_HEIGHT - 1 - asteroid->radius) {
        asteroid->y = SCREEN_HEIGHT - 1 - asteroid->radius;
        asteroid->y_speed = -asteroid->y_speed;
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