#include "player.h"

#include <ssd1306.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdbool.h>

#include "utils.h"

#include "systime.h"
#include "rgb_led.h"
#include "joystick.h"

player_t player;

#define NO_PIXEL 0x6510 
#define PLAYER_COLLISION_RADIUS 13

#define POWER_CONSTANT 0.07f
#define INERTIA_DECAY 0.85f

#define INVINCIBLE_MS 1000
#define SHIELD_MS 3000

#define SHIELD_PADDING 4

static const uint16_t spaceship_bitmap[] PROGMEM = {
	0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x2a9f, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x769d, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x769d, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x3c66, 0x3c66, 
	0x3c66, 0x3c66, 0x3c66, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x6510, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 
	0x7ecf, 0x7ecf, 0x3c66, 0x3c66, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x3c66, 
	0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x7ecf, 0x3c66, 0x3c66, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 
	0x3c66, 0x7ecf, 0x3c66, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 
	0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x7ecf, 0x3c66, 0x3c66, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x6510, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 
	0x3c66, 0x3c66, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 
	0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 0x3c66, 
	0x6510, 0x6510, 0x6510, 0x6510, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x86ff, 0x86ff, 0x769d, 0x769d, 0x769d, 0x86ff, 0x86ff, 0x769d, 0x769d, 0x769d, 0x86ff, 0x86ff, 0x769d, 
	0x769d, 0x769d, 0x86ff, 0x86ff, 0x769d, 0x769d, 0x769d, 0x6510, 0x769d, 0x769d, 0x86ff, 0x86ff, 0x769d, 0x769d, 0x769d, 0x86ff, 
	0x86ff, 0x769d, 0x769d, 0x769d, 0x86ff, 0x86ff, 0x769d, 0x769d, 0x769d, 0x86ff, 0x86ff, 0x769d, 0x769d, 0x6510, 0x6510, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x6510, 0x6510, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x6510, 0x6510, 0x6510, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x769d, 0x769d, 
	0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x6510, 0x6510, 0x6510, 
	0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 0x769d, 
	0x769d, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510, 0x6510
};

void player_init() {
    player.x = (SCREEN_WIDTH - 1) / 2;
    player.y = (SCREEN_HEIGHT - 1) / 2;

	player.x_speed_inertia = player.y_speed_inertia = 0;

	player.old_x_draw = round(player.x - (SPACESHIP_WIDTH - 1) / 2);
	player.old_y_draw = round(player.y - SPACESHIP_HEIGHT / 2);

    player.no_lives = 3;
    rgb_led_set_state(GREEN);

	player.no_shields = 3;

	player.invincible = player.shield = player.old_shield = false;
	player.display = true;
}

void player_update_pos(float delta_time) {
	player.x_speed_inertia *= INERTIA_DECAY; 
	if (fabs(player.x_speed_inertia) < 0.1f) {
		player.x_speed_inertia = 0;
	}

    player.y_speed_inertia *= INERTIA_DECAY;
	if (fabs(player.y_speed_inertia) < 0.1f) {
		player.y_speed_inertia = 0;
	}

	player.x += (joystick_get_x() + player.x_speed_inertia) * delta_time;
    player.y += (joystick_get_y() + player.y_speed_inertia) * delta_time;
}

void player_ckeck_update_invincibility() {
	if (!player.invincible) {
		return;
	}

	if (systime_get_ms() - player.invincible_ms_reference >= INVINCIBLE_MS && player.display) {
		player.invincible = false;
	}

	player.display = !player.display;
}

void player_check_update_shield() {
	player.old_shield = player.shield;

	if (joystick_is_sw_pressed() && player.no_shields > 0 && !player.shield) {
		player.no_shields--;
		player.shield = true;
		player.shield_ms_reference = systime_get_ms();
	}

	if (player.shield && systime_get_ms() - player.shield_ms_reference >= SHIELD_MS) {
		player.shield = false;
	}
}

void player_handle_collision_boundary() {
	float left = (SPACESHIP_WIDTH - 1) / 2;
	float right = SCREEN_WIDTH - 1 - (SPACESHIP_WIDTH - 1) / 2;
	float up = SPACESHIP_HEIGHT / 2 + UI_HEIGHT;
	float down = SCREEN_HEIGHT - 1 - (SPACESHIP_HEIGHT - 1) / 2;

	if (player.shield) {
		left += SHIELD_PADDING;
		right -= SHIELD_PADDING;
		up += SHIELD_PADDING;
		down -= SHIELD_PADDING;
	}

	if (player.x < left) {
		player.x = left;
		player.x_speed_inertia = 0;
	} else if (player.x > right) {
		player.x = right;
		player.x_speed_inertia = 0;
	}

	if (player.y < up) {
		player.y = up;
		player.y_speed_inertia = 0;
	} else if (player.y > down) {
		player.y = down;
		player.y_speed_inertia = 0;
	}
}

bool player_handle_collision_asteroids(asteroid_t *asteroids, uint8_t no_asteroids) {
	bool damaged = false;

	for (uint8_t i = 0; i < no_asteroids; i++) {
		asteroid_t *asteroid = &asteroids[i];

		if (player.shield) {
			float shield_left = player.x - (SPACESHIP_WIDTH - 1) / 2 - SHIELD_PADDING;
			float shield_right = player.x - (SPACESHIP_WIDTH - 1) / 2 + SPACESHIP_WIDTH - 1 + SHIELD_PADDING;

			float shield_up = player.y - SPACESHIP_HEIGHT / 2 - SHIELD_PADDING;
			float shield_down = player.y - SPACESHIP_HEIGHT / 2 + SPACESHIP_HEIGHT - 1 + SHIELD_PADDING;

			if (shield_up <= asteroid->y && asteroid->y <= shield_down) {
                if ((asteroid->x + asteroid->radius >= shield_left) && (asteroid->x < shield_left)) {
					if (asteroid->x_speed > 0) {
						asteroid->x = shield_left - asteroid->radius - 1;
						asteroid->x_speed = -asteroid->x_speed;
					} else {
						player.x = asteroid->x + asteroid->radius + 1 + SHIELD_PADDING + (SPACESHIP_WIDTH - 1) / 2.0f;
					}
                }

				if ((asteroid->x - asteroid->radius <= shield_right) && (asteroid->x > shield_right)) {
					if (asteroid->x_speed < 0) {
						asteroid->x = shield_right + asteroid->radius + 1;
						asteroid->x_speed = -asteroid->x_speed;
					} else {
						player.x = asteroid->x - asteroid->radius - 1 - SHIELD_PADDING - (SPACESHIP_WIDTH - 1) / 2.0f;
					}
				}
				continue;
			}
			
			if (shield_left <= asteroid->x && asteroid->x <= shield_right) {
				if ((asteroid->y + asteroid->radius >= shield_up) && (asteroid->y < shield_up)) {
					if (asteroid->y_speed > 0) {
						asteroid->y = shield_up - asteroid->radius - 1;
						asteroid->y_speed = -asteroid->y_speed;
					} else {
						player.y = asteroid->y + asteroid->radius + 1 + SHIELD_PADDING + SPACESHIP_HEIGHT / 2;
					}
				}

				if ((asteroid->y - asteroid->radius <= shield_down) && (asteroid->y > shield_down)) {
					if (asteroid->y_speed < 0) {
						asteroid->y = shield_down + asteroid->radius + 1;
						asteroid->y_speed = -asteroid->y_speed;
					} else {
						player.y = asteroid->y - asteroid->radius - 1 - SHIELD_PADDING - SPACESHIP_HEIGHT / 2;
					}
				}
				continue;
			}

			float radius2 = asteroid->radius * asteroid->radius;

			float dx = asteroid->x - shield_left;
			float dy = asteroid->y - shield_up;

			if (dx * dx + dy * dy <= radius2) {
				asteroid->x = shield_left - asteroid->radius - 1;
				asteroid->x_speed = -asteroid->x_speed;
				asteroid->y_speed = -asteroid->y_speed;
				continue;
			}

			dx = asteroid->x - shield_right;
			dy = asteroid->y - shield_up;

			if (dx * dx + dy * dy <= radius2) {
				asteroid->x = shield_right + asteroid->radius + 1;
				asteroid->x_speed = -asteroid->x_speed;
				asteroid->y_speed = -asteroid->y_speed;
				continue;
			}

			dx = asteroid->x - shield_left;
			dy = asteroid->y - shield_down;

			if (dx * dx + dy * dy <= radius2) {
				asteroid->x = shield_left - asteroid->radius - 1;
				asteroid->x_speed = -asteroid->x_speed;
				asteroid->y_speed = -asteroid->y_speed;
				continue;
			}

			dx = asteroid->x - shield_right;
			dy = asteroid->y - shield_down;

			if (dx * dx + dy * dy <= radius2) {
				asteroid->x = shield_right + asteroid->radius + 1;
				asteroid->x_speed = -asteroid->x_speed;
				asteroid->y_speed = -asteroid->y_speed;
				continue;
			}
		} else {
			float dx = player.x - asteroid->x;
			float dy = player.y - asteroid->y;

			float radius_sum = asteroid->radius + PLAYER_COLLISION_RADIUS;

			if (dx * dx + dy * dy > radius_sum * radius_sum) {
				continue;
			}

			float distance = hypot(dx, dy);
			if (distance < FLOAT_TOLERANCE) {
				distance = 1;
			}

			float speed = hypot(asteroid->x_speed, asteroid->y_speed);
			if (speed < FLOAT_TOLERANCE) {
				speed = 1;
			}

			float dot_product = (dx * asteroid->x_speed + dy * asteroid->y_speed) / (distance * speed);
			if (dot_product < 0.7f) {
				dot_product = 0.3f;
			}

			float radius2 = (float)asteroid->radius * (float)asteroid->radius;
			float power = POWER_CONSTANT * radius2 * speed * dot_product;

			player.x_speed_inertia = dx / distance * power;
			player.y_speed_inertia = dy / distance * power;

			if (!player.invincible) {
				player.invincible = true;
				player.invincible_ms_reference = systime_get_ms();
				player.display = false;

				player.no_lives--;

				if (player.no_lives == 2) {
					rgb_led_set_state(YELLOW);
				} else if (player.no_lives == 1) {
					rgb_led_set_state(BLINKING_RED);
				} else {
					rgb_led_set_state(OFF);
				}

				damaged = true;
			}
		}
	}

	return damaged;
}

void player_draw_init() {
	ssd1306_drawBitmap16(player.old_x_draw, player.old_y_draw, 23, 24, (uint8_t*)spaceship_bitmap);
}

static void draw_shield(int16_t x_draw, int16_t y_draw) {
	if (player.old_shield) {
		ssd1306_setColor(BACKGROUND_COLOR);
		ssd1306_drawRect16(
			player.old_x_draw - SHIELD_PADDING,
			player.old_y_draw - SHIELD_PADDING,
			player.old_x_draw + SPACESHIP_WIDTH - 1 + SHIELD_PADDING,
			player.old_y_draw + SPACESHIP_HEIGHT - 1 + SHIELD_PADDING
		);
	}

	if (player.shield) {
		ssd1306_setColor(SHIELD_COLOR);
		ssd1306_drawRect16(
			x_draw - SHIELD_PADDING,
			y_draw - SHIELD_PADDING,
			x_draw + SPACESHIP_WIDTH - 1 + SHIELD_PADDING,
			y_draw + SPACESHIP_HEIGHT - 1 + SHIELD_PADDING
		);
	}
}

void player_draw_diff() {
    int16_t x_draw = round(player.x - (SPACESHIP_WIDTH - 1) / 2);
	int16_t y_draw = round(player.y - SPACESHIP_HEIGHT / 2);

	draw_shield(x_draw, y_draw);

	if (x_draw == player.old_x_draw && y_draw == player.old_y_draw) {
		return;
	}

	ssd1306_drawBitmap16(x_draw, y_draw, 23, 24, (uint8_t*)spaceship_bitmap);
	ssd1306_setColor(BACKGROUND_COLOR);

	int16_t old_index = 0;
	int16_t index_offset = 23 * (y_draw - player.old_y_draw) + (x_draw - player.old_x_draw);

	for (int16_t old_y = player.old_y_draw; old_y < player.old_y_draw + SPACESHIP_HEIGHT; old_y++) {
		for (int16_t old_x = player.old_x_draw; old_x < player.old_x_draw + SPACESHIP_WIDTH; old_x++) {
			if (pgm_read_word(&spaceship_bitmap[old_index]) == NO_PIXEL) {
				old_index++;
				continue;
			}

			bool intersection = false;

			if (old_x >= x_draw && old_x < x_draw + SPACESHIP_WIDTH
				&& old_y >= y_draw && old_y < y_draw + SPACESHIP_HEIGHT) {
				int16_t index = old_index - index_offset;

				if (index >= 0 && index < SPACESHIP_WIDTH * SPACESHIP_HEIGHT
					&& pgm_read_word(&spaceship_bitmap[index]) != NO_PIXEL) {
					intersection = true;
				}
			}

			if (!intersection) {
				ssd1306_putPixel16(old_x, old_y);
			}

			old_index++;
		}
	}

	player.old_x_draw = x_draw;
	player.old_y_draw = y_draw;
}

void player_draw_invincible() {
	int16_t x_draw = round(player.x - (SPACESHIP_WIDTH - 1) / 2);
	int16_t y_draw = round(player.y - SPACESHIP_HEIGHT / 2);

	draw_shield(x_draw, y_draw);

	if (player.display) {
		ssd1306_drawBitmap16(x_draw, y_draw, SPACESHIP_WIDTH, SPACESHIP_HEIGHT, (uint8_t*)spaceship_bitmap);
	} else {
		ssd1306_setColor(BACKGROUND_COLOR);
		ssd1306_fillRect16(
			player.old_x_draw,
			player.old_y_draw,
			player.old_x_draw + SPACESHIP_WIDTH - 1,
			player.old_y_draw + SPACESHIP_HEIGHT - 1
		);
	}

	player.old_x_draw = x_draw;
	player.old_y_draw = y_draw;
}
