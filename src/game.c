#include <math.h>
#include "t3f/t3f.h"
#include "t3f/sound.h"
#include "t3f/rng.h"
#include "t3f/draw.h"
#include "instance.h"
#include "game.h"

void dot_shadow_text(ALLEGRO_FONT * font, ALLEGRO_COLOR color, ALLEGRO_COLOR shadow, float x, float y, float sx, float sy, int flags, const char * text);

/* initialize player (done once per turn and at new level) */
void dot_game_drop_player(void * data, int type)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int i;

	/* initialize player */
	app->game.player.ball.x = DOT_GAME_PLAYFIELD_WIDTH / 2;
	app->game.player.ball.y = DOT_GAME_PLAYFIELD_HEIGHT / 2;
	app->game.player.ball.z = 0.0;
	app->game.player.ball.a = ALLEGRO_PI / 2.0;
	app->game.player.ball.type = type;
	app->game.player.ball.active = true;
	app->game.player.ball.timer = 0;
	app->game.combo = 0;
	t3f_play_sample(app->sample[DOT_SAMPLE_START], 1.0, 0.0, 1.0);

	/* initialize game state */
	app->game.state = DOT_GAME_STATE_START;
	app->game.state_tick = 0;

	/* clear the area where the player is so we don't get cheap deaths */
	for(i = 0; i < DOT_GAME_MAX_BALLS; i++)
	{
		if(app->game.ball[i].active)
		{
			if(t3f_distance(app->game.ball[i].x, app->game.ball[i].y, DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2) < 64)
			{
				app->game.ball[i].a = atan2(app->game.ball[i].y - DOT_GAME_PLAYFIELD_HEIGHT / 2, app->game.ball[i].x - DOT_GAME_PLAYFIELD_WIDTH / 2);
				app->game.ball[i].vx = cos(app->game.ball[i].a) * app->game.ball[i].s;
				app->game.ball[i].vy = sin(app->game.ball[i].a) * app->game.ball[i].s;
			}
		}
	}
}

/* function to set up a new level */
void dot_game_setup_level(void * data, int level)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int i, j;
	int col = 0;
	int level_balls[DOT_GAME_MAX_LEVELS] =
	{
		32, 40, 48, 56, 64, 72, 80, 88, 96, 104
	};

	app->game.level = level < 10 ? level : 10;

	/* initialize balls */
	memset(app->game.ball, 0, sizeof(DOT_BALL) * DOT_GAME_MAX_BALLS);
	for(i = 0; i < level_balls[app->game.level]; i++)
	{
		app->game.ball[i].r = 8.0;
		app->game.ball[i].x = t3f_drand(&app->rng_state) * ((float)(DOT_GAME_PLAYFIELD_WIDTH) - app->game.ball[i].r * 2.0) + app->game.ball[i].r;
		app->game.ball[i].y = t3f_drand(&app->rng_state) * ((float)(DOT_GAME_PLAYFIELD_HEIGHT) - app->game.ball[i].r * 2.0) + app->game.ball[i].r;
		app->game.ball[i].z = 0;
		app->game.ball[i].a = t3f_drand(&app->rng_state) * ALLEGRO_PI * 2.0;
		app->game.ball[i].s = t3f_drand(&app->rng_state) * 0.75 + 0.25;
		app->game.ball[i].vx = cos(app->game.ball[i].a) * app->game.ball[i].s;
		app->game.ball[i].vy = sin(app->game.ball[i].a) * app->game.ball[i].s;
		app->game.ball[i].type = col;
		col++;
		if(col > 5)
		{
			col = 0;
		}
		app->game.ball[i].active = true;
	}

	/* add black balls */
	for(j = i; j < i + app->game.level + 3; j++)
	{
		app->game.ball[j].r = 8.0;
		app->game.ball[j].x = t3f_drand(&app->rng_state) * ((float)(DOT_GAME_PLAYFIELD_WIDTH) - app->game.ball[j].r * 2.0) + app->game.ball[i].r;
		app->game.ball[j].y = t3f_drand(&app->rng_state) * ((float)(DOT_GAME_PLAYFIELD_HEIGHT) - app->game.ball[j].r * 2.0) + app->game.ball[i].r;
		app->game.ball[j].z = 0;
		app->game.ball[j].a = t3f_drand(&app->rng_state) * ALLEGRO_PI * 2.0;
		app->game.ball[j].s = t3f_drand(&app->rng_state) * 0.75 + 0.25;
		app->game.ball[j].vx = cos(app->game.ball[j].a) * app->game.ball[j].s;
		app->game.ball[j].vy = sin(app->game.ball[j].a) * app->game.ball[j].s;
		app->game.ball[j].type = 6;
		app->game.ball[j].active = true;
	}

	/* drop the player with a random color */
	dot_game_drop_player(data, t3f_rand(&app->rng_state) % 6);
	app->game.player.ball.r = 8.0;

	app->game.timer = 0;
}

/* start the game from level 0 */
void dot_game_initialize(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	dot_game_setup_level(data, 0);
	app->game.score = 0;
	app->game.combo = 0;
	app->game.lives = 3;
	t3f_srand(&app->rng_state, time(0));
	app->state = DOT_STATE_GAME;
}

/* finish the game */
void dot_game_exit(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	char buf[256] = {0};

	/* save high score */
	sprintf(buf, "%d", app->game.high_score);
	al_set_config_value(t3f_config, "Game Data", "High Score", buf);

	/* go back to intro */
	app->state = DOT_STATE_INTRO;
}

/* function to add points to the score
 * used when combo timer reaches 0, level is completed, or player loses */
void dot_game_accumulate_score(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->game.score += app->game.ascore * app->game.combo;
	if(app->game.score > app->game.high_score)
	{
		app->game.high_score = app->game.score;
	}
}

/* return the number of colored balls left */
int dot_game_move_balls(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	float speed = 1.0;
	int colored = 0;
	int i;

	/* set the speed of the balls */
	if(app->game.state == DOT_GAME_STATE_START)
	{
		speed = 0.25;
	}
	else
	{
		speed = 1.0 + (float)app->game.timer / 3600.0;
		if(speed > 3.0)
		{
			speed = 3.0;
		}
	}

	/* move the balls */
	for(i = 0; i < DOT_GAME_MAX_BALLS; i++)
	{
		if(app->game.ball[i].active)
		{
			app->game.ball[i].x += app->game.ball[i].vx * speed;
			if(app->game.ball[i].x - app->game.ball[i].r < 0.0)
			{
				app->game.ball[i].vx = -app->game.ball[i].vx;
			}
			if(app->game.ball[i].x + app->game.ball[i].r >= DOT_GAME_PLAYFIELD_WIDTH)
			{
				app->game.ball[i].vx = -app->game.ball[i].vx;
			}
			app->game.ball[i].y += app->game.ball[i].vy * speed;
			if(app->game.ball[i].y - app->game.ball[i].r < 0.0)
			{
				app->game.ball[i].vy = -app->game.ball[i].vy;
			}
			if(app->game.ball[i].y + app->game.ball[i].r >= DOT_GAME_PLAYFIELD_HEIGHT)
			{
				app->game.ball[i].vy = -app->game.ball[i].vy;
			}
			if(app->game.ball[i].type != DOT_BITMAP_BALL_BLACK)
			{
				colored++;
			}
		}
	}
	return colored;
}

/* figure out the direction to rotate to move the angle from a1 to a2 */
static float get_angle_dir(float a1, float a2)
{
	float distance = fabs(a1 - a2);
	if(distance < ALLEGRO_PI)
	{
		if(a1 < a2)
		{
			return -1.0;
		}
		else
		{
			return 1.0;
		}
	}
	if(a1 < a2)
	{
		return 1.0;
	}
	else
	{
		return -1.0;
	}
}

/* handle player movement */
void dot_game_move_player(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int i, j;
	float ox, oy, target_angle;

	if(app->game.player.ball.active)
	{
		ox = app->game.player.ball.x;
		oy = app->game.player.ball.y;

		if(app->touch_id >= 0)
		{
			if(app->game.player.lost_touch)
			{
				if(t3f_distance(app->touch_x, app->touch_y, app->game.player.ball.x, app->game.player.ball.y) < DOT_GAME_GRAB_SPOT_SIZE)
				{
					app->game.player.lost_touch = false;
				}
			}
			if(!app->game.player.lost_touch)
			{
				app->game.player.ball.x = app->touch_x;
				app->game.player.ball.y = app->touch_y;
			}
		}
		else
		{
			app->game.player.lost_touch = true;
			app->game.state = DOT_GAME_STATE_PAUSE;
		}

		/* prevent player from moving past the edge */
		if(app->game.player.ball.x - app->game.player.ball.r < 0.0)
		{
			app->game.player.ball.x = app->game.player.ball.r;
		}
		if(app->game.player.ball.x + app->game.player.ball.r >= DOT_GAME_PLAYFIELD_WIDTH)
		{
			app->game.player.ball.x = DOT_GAME_PLAYFIELD_WIDTH - app->game.player.ball.r;
		}
		if(app->game.player.ball.y - app->game.player.ball.r < 0.0)
		{
			app->game.player.ball.y = app->game.player.ball.r;
		}
		if(app->game.player.ball.y + app->game.player.ball.r >= DOT_GAME_PLAYFIELD_HEIGHT)
		{
			app->game.player.ball.y = DOT_GAME_PLAYFIELD_HEIGHT - app->game.player.ball.r;
		}

		/* if the player has moved, change the angle of the character */
		if((int)ox != (int)app->game.player.ball.x || (int)oy != (int)app->game.player.ball.y)
		{
			target_angle = atan2(oy - app->game.player.ball.y, ox - app->game.player.ball.x);
			if(fabs(target_angle - app->game.player.ball.a) > ALLEGRO_PI / 16.0)
			{
				app->game.player.ball.a += (ALLEGRO_PI / 16.0) * get_angle_dir(target_angle, app->game.player.ball.a);
				if(app->game.player.ball.a > ALLEGRO_PI)
				{
					app->game.player.ball.a -= ALLEGRO_PI * 2.0;
				}
				if(app->game.player.ball.a < -ALLEGRO_PI)
				{
					app->game.player.ball.a += ALLEGRO_PI * 2.0;
				}
			}
		}

		/* handle combo timer */
		if(app->game.combo > 0)
		{
			app->game.player.ball.timer++;
			if(app->game.player.ball.timer >= DOT_GAME_COMBO_TIME)
			{
				dot_game_accumulate_score(data);
				app->game.player.ball.timer = 0;
				app->game.combo = 0;
				app->game.ascore = 0;
			}
		}

		/* see if player ball hits any other balls */
		for(i = 0; i < DOT_GAME_MAX_BALLS; i++)
		{
			if(app->game.ball[i].active)
			{
				if(t3f_distance(app->game.ball[i].x, app->game.ball[i].y, app->game.player.ball.x, app->game.player.ball.y) < app->game.player.ball.r + app->game.ball[i].r)
				{

					/* hitting the same color gives you points and increases your combo */
					if(app->game.ball[i].type == app->game.player.ball.type)
					{
						t3f_play_sample(app->sample[DOT_SAMPLE_GRAB], 1.0, 0.0, 1.0);
						app->game.ball[i].active = false;
						app->game.ascore += 50;
						if(app->game.player.ball.timer < DOT_GAME_COMBO_TIME)
						{
							app->game.combo++;
						}
						app->game.player.ball.timer = 0;
						app->game.player.ball.r += 0.5;
						for(j = 0; j < DOT_GAME_MAX_BALLS; j++)
						{
							if(app->game.ball[j].active && app->game.ball[j].type != DOT_BITMAP_BALL_BLACK && app->game.ball[j].type != app->game.player.ball.type)
							{
								app->game.ball[j].type = app->game.player.ball.type;
								break;
							}
						}
					}

					/* hitting other color kills you */
					else
					{
						if(app->touch_id >= 0)
						{
							t3f_touch[app->touch_id].active = false;
						}
						t3f_play_sample(app->sample[DOT_SAMPLE_LOSE], 1.0, 0.0, 1.0);
						dot_game_accumulate_score(data);
						app->game.lives--;

						/* change ball color to match the ball that is hit unless it is black */
						if(app->game.ball[i].type != DOT_BITMAP_BALL_BLACK)
						{
							if(app->game.lives > 0)
							{
								dot_game_drop_player(data, app->game.ball[i].type);
							}
							else
							{
								dot_game_exit(data);
							}
						}
						else
						{
							if(app->game.lives > 0)
							{
								dot_game_drop_player(data, app->game.player.ball.type);
							}
							else
							{
								dot_game_exit(data);
							}
						}
						break;
					}
				}
			}
		}
	}
}

/* the main game logic function */
void dot_game_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int colored = 0;

	switch(app->game.state)
	{

		/* balls move slow for a few seconds so player can get ready */
		case DOT_GAME_STATE_START:
		{
			app->game.state_tick++;
			if(app->touch_id >= 0)
			{
				t3f_play_sample(app->sample[DOT_SAMPLE_GO], 1.0, 0.0, 1.0);
				app->game.state = DOT_GAME_STATE_PLAY;
				app->game.state_tick = 0;
				app->game.player.lost_touch = false;
				app->game.player.ball.active = true;
//				al_set_mouse_xy(t3f_display, DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2);
				t3f_mouse_x = DOT_GAME_PLAYFIELD_WIDTH / 2;
				t3f_mouse_y = DOT_GAME_PLAYFIELD_HEIGHT / 2;
			}

			/* handle ball logic */
			colored = dot_game_move_balls(data);
			break;
		}

		case DOT_GAME_STATE_PAUSE:
		{
			if(app->touch_id >= 0 && t3f_distance(app->touch_x, app->touch_y, app->game.player.ball.x, app->game.player.ball.y) < DOT_GAME_GRAB_SPOT_SIZE)
			{
				app->game.state = DOT_GAME_STATE_PLAY;
			}
			break;
		}

		/* normal game state */
		default:
		{
			app->game.timer++;

			/* handle ball logic */
			colored = dot_game_move_balls(data);

			/* move player */
			dot_game_move_player(data);

			/* move on to next level */
			if(colored == 0)
			{
				dot_game_accumulate_score(data);
				if(app->game.level < 9)
				{
					if(app->touch_id >= 0)
					{
						t3f_touch[app->touch_id].active = false;
					}
					dot_game_setup_level(data, app->game.level + 1);
				}
				else
				{
					dot_game_exit(data);
				}
			}
			break;
		}
	}
	if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
	{
		app->state = DOT_STATE_INTRO;
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
		t3f_key[ALLEGRO_KEY_BACK] = 0;
	}
}

/* render the HUD */
void dot_game_render_hud(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	char buffer[256] = {0};
	ALLEGRO_COLOR shadow = al_map_rgba_f(0.0, 0.0, 0.0, 0.25);

	al_draw_filled_rectangle(0, DOT_GAME_PLAYFIELD_HEIGHT, 540, DOT_GAME_PLAYFIELD_HEIGHT + 80, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
	al_hold_bitmap_drawing(true);
//	al_draw_bitmap(app->bitmap[DOT_BITMAP_HUD], 480.0, 0.0, 0);
	sprintf(buffer, "Score");
	dot_shadow_text(app->font, t3f_color_white, shadow, 16 + 48 + 16, 440 + 8 + 9, 2, 2, 0, buffer);
	sprintf(buffer, "  %07d", app->game.score);
	dot_shadow_text(app->font, t3f_color_white, shadow, 16 + 48 + 16, 440 + 8 + 9 + 24, 2, 2, 0, buffer);
	sprintf(buffer, "High");
	dot_shadow_text(app->font, t3f_color_white, shadow, 16 + 48 + 16 + 160, 440 + 8 + 9, 2, 2, 0, buffer);
	sprintf(buffer, "  %07d", app->game.high_score);
	dot_shadow_text(app->font, t3f_color_white, shadow, 16 + 48 + 16 + 160, 440 + 8 + 9 + 24, 2, 2, 0, buffer);
/*	sprintf(buffer, "Combo");
	dot_shadow_text(app->font, t3f_color_black, shadow, 480 + 8, 88, 2, 2, 0, buffer);
	sprintf(buffer, "      %03d", app->game.combo);
	dot_shadow_text(app->font, t3f_color_black, shadow, 480 + 8, 104, 2, 2, 0, buffer);
	sprintf(buffer, "Level");
	dot_shadow_text(app->font, t3f_color_black, shadow, 480 + 8, 128, 2, 2, 0, buffer);
	sprintf(buffer, "       %02d", app->game.level);
	dot_shadow_text(app->font, t3f_color_black, shadow, 480 + 8, 144, 2, 2, 0, buffer);
	sprintf(buffer, "Lives");
	dot_shadow_text(app->font, t3f_color_black, shadow, 480 + 8, 168, 2, 2, 0, buffer);
	sprintf(buffer, "       %02d", app->game.lives);
	dot_shadow_text(app->font, t3f_color_black, shadow, 480 + 8, 184, 2, 2, 0, buffer); */

	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], shadow, 16 + 2, 440 + 16 + 2, 0, 48, 48, 0);
	al_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], 0, 0, 16, 16, 16, 440 + 16, 48, 48, 0);
	al_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_EYES], 8.0, 8.0, 16 + 24, 440 + 16 + 24, 3, 3, ALLEGRO_PI / 2.0, 0);
	al_hold_bitmap_drawing(false);
}

/* main game render function */
void dot_game_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int i;
	ALLEGRO_COLOR player_color = t3f_color_white;
	float rgb = 0.5 - (float)app->game.level / 24.0;

	al_clear_to_color(al_map_rgb_f(rgb, rgb, rgb));
	al_hold_bitmap_drawing(true);
	for(i = 0; i < DOT_GAME_MAX_BALLS; i++)
	{
		if(app->game.ball[i].active)
		{
			t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.ball[i].type], t3f_color_white, app->game.ball[i].x - app->game.ball[i].r, app->game.ball[i].y - app->game.ball[i].r, app->game.ball[i].z, app->game.ball[i].r * 2.0, app->game.ball[i].r * 2.0, 0);
		}
	}
	for(i = 0; i < DOT_GAME_MAX_BALLS; i++)
	{
		if(app->game.ball[i].active)
		{
			t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.ball[i].type], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), app->game.ball[i].x - app->game.ball[i].r, 960 - DOT_GAME_PLAYFIELD_HEIGHT + app->game.ball[i].y - app->game.ball[i].r, app->game.ball[i].z, app->game.ball[i].r * 2.0, app->game.ball[i].r * 2.0, 0);
		}
	}
	if(app->game.player.ball.active)
	{
		if(app->game.state == DOT_GAME_STATE_START)
		{
			player_color = al_map_rgba_f(1.0 * 0.5, 1.0 * 0.5, 1.0 * 0.5, 0.5);
		}
		t3f_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], player_color, 8.0, 8.0, app->game.player.ball.x, app->game.player.ball.y, app->game.player.ball.z, 0.0, app->game.player.ball.r / (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2), app->game.player.ball.r / (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2), 0);
		t3f_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_EYES], player_color, 8.0, 8.0, app->game.player.ball.x, app->game.player.ball.y, app->game.player.ball.z, app->game.player.ball.a, app->game.player.ball.r / (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2), app->game.player.ball.r / (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2), 0);
		t3f_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], al_map_rgba_f(0.5, 0.5, 0.5, 0.5), 8.0, 8.0, app->game.player.ball.x, 960 - DOT_GAME_PLAYFIELD_HEIGHT + app->game.player.ball.y, app->game.player.ball.z, 0.0, app->game.player.ball.r / (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2), app->game.player.ball.r / (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2), 0);
	}
	al_hold_bitmap_drawing(false);
	dot_game_render_hud(data);
	if(app->game.state == DOT_GAME_STATE_PAUSE)
	{
		al_draw_filled_rectangle(0.0, 0.0, t3f_virtual_display_width, t3f_virtual_display_height, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		al_draw_filled_circle(app->game.player.ball.x, 960 - DOT_GAME_PLAYFIELD_HEIGHT + app->game.player.ball.y, DOT_GAME_GRAB_SPOT_SIZE, al_map_rgba_f(0.5, 0.5, 0.5, 0.5));
	}
}
