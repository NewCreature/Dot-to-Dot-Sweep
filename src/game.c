#include <math.h>
#include "t3f/t3f.h"
#include "t3f/sound.h"
#include "t3f/rng.h"
#include "t3f/draw.h"
#include "t3net/leaderboard.h"
#include "instance.h"
#include "game.h"
#include "text.h"
#include "color.h"

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
	int num_balls = DOT_GAME_LEVEL_BASE_BALLS + level * DOT_GAME_LEVEL_BALLS_INC;
	int num_black_balls = DOT_GAME_LEVEL_BASE_BLACK_BALLS + (level / 2) * DOT_GAME_LEVEL_BLACK_BALLS_INC;

	/* initialize balls */
	memset(app->game.ball, 0, sizeof(DOT_BALL) * DOT_GAME_MAX_BALLS);
	for(i = 0; i < num_balls && i < DOT_GAME_MAX_BALLS; i++)
	{
		app->game.ball[i].r = DOT_GAME_BALL_SIZE;
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
	for(j = i; j < i + num_black_balls && j < DOT_GAME_MAX_BALLS; j++)
	{
		app->game.ball[j].r = DOT_GAME_BALL_SIZE;
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
	t3f_play_sample(app->sample[DOT_SAMPLE_START], 1.0, 0.0, 1.0);
	app->game.player.ball.r = 16.0;

	app->game.level = level;
	app->game.speed = DOT_GAME_LEVEL_BASE_SPEED;
	app->game.speed_inc = DOT_GAME_LEVEL_TOP_SPEED / (float)num_balls;
	app->game.level_start = true;
}

/* start the game from level 0 */
void dot_game_initialize(void * data, bool demo_seed)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(demo_seed)
    {
        t3f_srand(&app->rng_state, app->demo_seed);
    }
    else
    {
        t3f_srand(&app->rng_state, time(0));
    }
	dot_game_setup_level(data, 0);
	app->game.score = 0;
	app->game.combo = 0;
	app->game.lives = 3;
	app->game.shield.active = false;
	if(app->music_enabled)
	{
		t3f_play_music("data/music/going_for_it.xm");
	}
	app->game.tick = 0;
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

	/* upload score */
	if(app->upload_scores)
	{
		sprintf(buf, "%d", app->game.level + 1);
		al_stop_timer(t3f_timer);
		t3net_upload_score(DOT_LEADERBOARD_SUBMIT_URL, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "normal", "none", app->user_name, app->game.score, buf);
		al_resume_timer(t3f_timer);
	}

	if(app->music_enabled)
	{
		t3f_stop_music();
	}

	/* go back to intro */
	app->state = DOT_STATE_INTRO;
}

static int dot_game_get_combo_score(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	return app->game.ascore * app->game.combo;
}

static float dot_spread_effect_particle(int pos, int max, float size)
{
	return ((float)(pos + 1) / (float)max) * size - size / 2.0;
}

static void dot_game_create_score_effect(void * data, float x, float y, int number)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	char buf[16] = {0};
	int i, j;
	ALLEGRO_COLOR c;
	unsigned char r, g, b, a;
	float ox;
	float w, h;

	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM);
	al_set_target_bitmap(app->bitmap[DOT_BITMAP_SCRATCH]);
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
	sprintf(buf, "%d", number);
	al_draw_text(app->font[DOT_FONT_16], t3f_color_white, 0, 0, 0, buf);
	al_restore_state(&old_state);
	al_lock_bitmap(app->bitmap[DOT_BITMAP_SCRATCH], ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
	ox = al_get_text_width(app->font[DOT_FONT_16], buf) / 2;
	w = al_get_text_width(app->font[DOT_FONT_16], buf);
	h = al_get_font_line_height(app->font[DOT_FONT_16]);
	for(i = 0; i < w; i++)
	{
		for(j = 0; j < h; j++)
		{
			c = al_get_pixel(app->bitmap[DOT_BITMAP_SCRATCH], i, j);
			al_unmap_rgba(c, &r, &g, &b, &a);
			if(a > 0)
			{
				dot_create_particle(&app->particle[app->current_particle], x + (float)i - ox, y + j, 0.0, dot_spread_effect_particle(i, w, strlen(buf) * 2.5), dot_spread_effect_particle(j, h, 4.0), -10.0, 0.0, 3.0, 45, app->bitmap[DOT_BITMAP_PARTICLE], c);
				app->current_particle++;
				if(app->current_particle >= DOT_MAX_PARTICLES)
				{
					app->current_particle = 0;
				}
			}
		}
	}
	al_unlock_bitmap(app->bitmap[DOT_BITMAP_SCRATCH]);
}

static void dot_game_create_splash_effect(void * data, float x, float y, float r, ALLEGRO_COLOR color)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;
	float ga, gx, gy;

	for(i = 0; i < r * 8.0; i++)
	{
		ga = t3f_drand(&app->rng_state) * ALLEGRO_PI * 2.0;
		gx = cos(ga) * t3f_drand(&app->rng_state) * r * t3f_drand(&app->rng_state);
		gy = sin(gy) * t3f_drand(&app->rng_state) * r * t3f_drand(&app->rng_state);
		dot_create_particle(&app->particle[app->current_particle], x + gx, y + gy, 0.0, cos(ga) * t3f_drand(&app->rng_state), sin(ga) * t3f_drand(&app->rng_state), t3f_drand(&app->rng_state) * -5.0 - 5.0, 0.5, 5.0, 30, app->bitmap[DOT_BITMAP_PARTICLE], color);
		app->current_particle++;
		if(app->current_particle >= DOT_MAX_PARTICLES)
		{
			app->current_particle = 0;
		}
	}
}

/* function to add points to the score
 * used when combo timer reaches 0, level is completed, or player loses */
void dot_game_accumulate_score(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int c;

	c = dot_game_get_combo_score(data);
	if(c > 0)
	{
		app->game.score += c;
		if(app->game.score > app->game.high_score)
		{
			app->game.high_score = app->game.score;
		}
		dot_game_create_score_effect(data, app->game.player.ball.x, app->game.player.ball.y - app->game.player.ball.r - 16.0 - 8.0, c);
	}
}

void dot_game_ball_shield_reaction(void * data, int i)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	float angle;
	float angle_c, angle_s;
	float distance;

	angle = atan2(app->game.ball[i].y - app->game.shield.y, app->game.ball[i].x - app->game.shield.x);
	angle_c = cos(angle);
	angle_s = sin(angle);
	distance = app->game.shield.r + app->game.ball[i].r;
	app->game.ball[i].x = app->game.shield.x + angle_c * distance;
	app->game.ball[i].y = app->game.shield.y + angle_s * distance;
	app->game.ball[i].vx = app->game.ball[i].s * angle_c;
	app->game.ball[i].vy = app->game.ball[i].s * angle_s;
}

void dot_game_move_ball(void * data, int i)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	float d;

	app->game.ball[i].x += app->game.ball[i].vx * app->game.speed;
	if(app->game.ball[i].x - app->game.ball[i].r < 0.0)
	{
		app->game.ball[i].vx = -app->game.ball[i].vx;
	}
	if(app->game.ball[i].x + app->game.ball[i].r >= DOT_GAME_PLAYFIELD_WIDTH)
	{
		app->game.ball[i].vx = -app->game.ball[i].vx;
	}
	app->game.ball[i].y += app->game.ball[i].vy * app->game.speed;
	if(app->game.ball[i].y - app->game.ball[i].r < 0.0)
	{
		app->game.ball[i].vy = -app->game.ball[i].vy;
	}
	if(app->game.ball[i].y + app->game.ball[i].r >= DOT_GAME_PLAYFIELD_HEIGHT)
	{
		app->game.ball[i].vy = -app->game.ball[i].vy;
	}
	if(app->game.shield.active)
	{
		d = t3f_distance(app->game.ball[i].x, app->game.ball[i].y, app->game.shield.x, app->game.shield.y);
		if(d < app->game.ball[i].r + app->game.shield.r)
		{
			dot_game_ball_shield_reaction(data, i);
		}
	}
}

/* return the number of colored balls left */
int dot_game_move_balls(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int colored = 0;
	int i;

	/* move the balls */
	for(i = 0; i < DOT_GAME_MAX_BALLS; i++)
	{
		if(app->game.ball[i].active)
		{
			dot_game_move_ball(data, i);
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

void dot_game_check_player_collisions(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i, j;

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
//						app->game.player.ball.r += 0.5;
					for(j = 0; j < DOT_GAME_MAX_BALLS; j++)
					{
						if(app->game.ball[j].active && app->game.ball[j].type != DOT_BITMAP_BALL_BLACK && app->game.ball[j].type != app->game.player.ball.type)
						{
							app->game.ball[j].type = app->game.player.ball.type;
							break;
						}
					}
					app->game.speed += app->game.speed_inc;
					dot_game_create_splash_effect(data, app->game.ball[i].x, app->game.ball[i].y, app->game.ball[i].r, app->color[app->game.ball[i].type]);
				}

				/* hitting other color kills you */
				else
				{
					if(app->touch_id >= 0)
					{
						t3f_touch[app->touch_id].active = false;
					}
					dot_game_create_splash_effect(data, app->game.player.ball.x, app->game.player.ball.y, app->game.player.ball.r, app->color[app->game.player.ball.type]);
					t3f_play_sample(app->sample[DOT_SAMPLE_LOSE], 1.0, 0.0, 1.0);
					dot_game_accumulate_score(data);
					app->game.combo = 0;
					app->game.ascore = 0;
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
							app->game.player.ball.active = false;
							app->game.state = DOT_GAME_STATE_DONE;
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
							app->game.player.ball.active = false;
							app->game.state = DOT_GAME_STATE_DONE;
						}
					}
					break;
				}
			}
		}
	}
}

/* handle player movement */
void dot_game_move_player(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

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
				app->game.player.ball.x = app->touch_x + app->game.player.touch_offset_x;
				app->game.player.ball.y = app->touch_y + app->game.player.touch_offset_y;
			}
			if(app->game.player.want_shield)
			{
				app->game.shield.x = app->game.player.ball.x;
				app->game.shield.y = app->game.player.ball.y;
				app->game.shield.r = app->game.player.ball.r + 1.0;
				app->game.shield.active = true;
				app->game.player.want_shield = false;
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
				t3f_play_sample(app->sample[DOT_SAMPLE_SCORE], 1.0, 0.0, 1.0);
			}
		}
	}
}

void dot_game_shield_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->game.shield.active)
	{
		app->game.shield.r += 1.5;
		if(app->game.shield.r >= 96.0)
		{
			app->game.shield.active = false;
		}
	}
}

/* the main game logic function */
void dot_game_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int colored = 0;
	int i;

	switch(app->game.state)
	{

		/* balls move slow for a few seconds so player can get ready */
		case DOT_GAME_STATE_START:
		{
			app->game.state_tick++;
			if(app->touch_id >= 0)
			{
				if(app->touch_x >= DOT_GAME_TOUCH_START_X && app->touch_x < DOT_GAME_TOUCH_END_X && app->touch_y >= DOT_GAME_TOUCH_START_Y && app->touch_y < DOT_GAME_TOUCH_END_Y)
				{
					t3f_play_sample(app->sample[DOT_SAMPLE_GO], 1.0, 0.0, 1.0);
					app->game.state = DOT_GAME_STATE_PLAY;
					app->game.state_tick = 0;
					app->game.player.lost_touch = false;
					app->game.player.ball.active = true;
					app->game.player.want_shield = true;
					app->game.player.touch_offset_x = 0;
					app->game.player.touch_offset_y = 0;
					app->game.level_start = false;
				}
			}

			/* handle ball logic */
			colored = dot_game_move_balls(data);
			break;
		}

		case DOT_GAME_STATE_PAUSE:
		{
			if(app->touch_id >= 0 && t3f_distance(app->touch_x, app->touch_y, app->game.player.ball.x, app->game.player.ball.y) < DOT_GAME_GRAB_SPOT_SIZE)
			{
				app->game.player.touch_offset_x = app->game.player.ball.x - app->touch_x;
				app->game.player.touch_offset_y = app->game.player.ball.y - app->touch_y;
				app->game.state = DOT_GAME_STATE_PLAY;
			}
			break;
		}

		case DOT_GAME_STATE_DONE:
		{
			for(i = 0; i < DOT_MAX_PARTICLES; i++)
			{
				if(app->particle[i].active)
				{
					break;
				}
			}
			if(i == DOT_MAX_PARTICLES)
			{
				dot_game_exit(data);
			}
			break;
		}

		/* normal game state */
		default:
		{
			/* handle shield logic */
			dot_game_shield_logic(data);

			/* move player */
			dot_game_move_player(data);

			/* handle ball logic */
			colored = dot_game_move_balls(data);

			dot_game_check_player_collisions(data);

			/* move on to next level */
			if(colored == 0)
			{
				dot_game_accumulate_score(data);
				if(app->touch_id >= 0)
				{
					t3f_touch[app->touch_id].active = false;
				}
				dot_game_setup_level(data, app->game.level + 1);
				app->game.combo = 0;
				app->game.ascore = 0;
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
	app->game.tick++;
}

/* render the HUD */
void dot_game_render_hud(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	char buffer[256] = {0};
	ALLEGRO_COLOR shadow = al_map_rgba_f(0.0, 0.0, 0.0, 0.25);
	bool held = al_is_bitmap_drawing_held();

	if(held)
	{
		al_hold_bitmap_drawing(false);
	}

	al_draw_filled_rectangle(0, DOT_GAME_PLAYFIELD_HEIGHT, 540, DOT_GAME_PLAYFIELD_HEIGHT + 80, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
	al_hold_bitmap_drawing(true);
//	al_draw_bitmap(app->bitmap[DOT_BITMAP_HUD], 480.0, 0.0, 0);
	sprintf(buffer, "Score");
	dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, shadow, 16 + 48 + 16, 440 + 8 + 9, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, buffer);
	sprintf(buffer, "  %07d", app->game.score);
	dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, shadow, 16 + 48 + 16, 440 + 8 + 9 + 24, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, buffer);
	sprintf(buffer, "High");
	dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, shadow, 16 + 48 + 16 + 160, 440 + 8 + 9, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, buffer);
	sprintf(buffer, "  %07d", app->game.high_score);
	dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, shadow, 16 + 48 + 16 + 160, 440 + 8 + 9 + 24, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, buffer);

	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], shadow, 16 + DOT_SHADOW_OX, 440 + 16 + DOT_SHADOW_OY, 0, 48, 48, 0);
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], t3f_color_white, 16, 440 + 16, 0, 48, 48, 0);
	al_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_EYES], 8.0, 8.0, 16 + 24, 440 + 16 + 24, 3, 3, ALLEGRO_PI / 2.0, 0);
	al_hold_bitmap_drawing(false);
	al_hold_bitmap_drawing(held);
}

static void dot_create_grab_spot_effect(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	float s;
	float sx = 512.0 / (float)t3f_virtual_display_width;
	bool held = al_is_bitmap_drawing_held();

	if(held)
	{
		al_hold_bitmap_drawing(false);
	}

	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM | ALLEGRO_STATE_BLENDER);
	al_set_target_bitmap(app->bitmap[DOT_BITMAP_SCRATCH]);
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 1.0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
	s = DOT_GAME_GRAB_SPOT_SIZE;
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_COMBO], al_map_rgba_f(0.0, 0.0, 0.0, 1.0), (float)(app->game.player.ball.x - DOT_GAME_GRAB_SPOT_SIZE) * sx, app->game.player.ball.y - DOT_GAME_GRAB_SPOT_SIZE, 0.0, (s * 2.0) * sx, s * 2, 0);
	al_restore_state(&old_state);
	al_hold_bitmap_drawing(held);
}

static void dot_create_touch_dots_effect(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	float s;
	float sx = 512.0 / (float)t3f_virtual_display_width;
	int i;
	bool held = al_is_bitmap_drawing_held();

	if(held)
	{
		al_hold_bitmap_drawing(false);
	}
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM);
	al_set_target_bitmap(app->bitmap[DOT_BITMAP_SCRATCH]);
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
	al_hold_bitmap_drawing(true);
	for(i = 0; i < DOT_GAME_MAX_BALLS; i++)
	{
		if(app->game.ball[i].active)
		{
			t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.ball[i].type], al_map_rgba_f(1.0, 1.0, 1.0, 1.0), (app->game.ball[i].x - app->game.ball[i].r) * sx, app->game.ball[i].y - app->game.ball[i].r, app->game.ball[i].z, (app->game.ball[i].r * 2.0) * sx, app->game.ball[i].r * 2.0, 0);
		}
	}
	al_hold_bitmap_drawing(false);
	al_restore_state(&old_state);
	al_hold_bitmap_drawing(held);
}

static void dot_create_touch_start_effect(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	float s;
	float sx = 512.0 / (float)t3f_virtual_display_width;
	bool held = al_is_bitmap_drawing_held();

	if(held)
	{
		al_hold_bitmap_drawing(false);
	}
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM | ALLEGRO_STATE_BLENDER);
	al_set_target_bitmap(app->bitmap[DOT_BITMAP_SCRATCH]);
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 1.0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
	s = DOT_GAME_GRAB_SPOT_SIZE;
	al_draw_filled_rectangle(DOT_GAME_TOUCH_START_X * sx, DOT_GAME_TOUCH_START_Y, DOT_GAME_TOUCH_END_X * sx, DOT_GAME_TOUCH_END_Y, al_map_rgba_f(1.0, 1.0, 1.0, 1.0));
	al_restore_state(&old_state);
	al_hold_bitmap_drawing(held);
}

/* main game render function */
void dot_game_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char buf[16] = {0};

	int i;
	ALLEGRO_COLOR player_color = t3f_color_white;
	ALLEGRO_COLOR text_color = t3f_color_white;
	float rgb = 0.75 - (float)app->game.level / 24.0;
	float c = (float)app->game.player.ball.timer / (float)DOT_GAME_COMBO_TIME;
	float s;
	float cx, cy, ecx, ecy;
	if(rgb < 0.0)
	{
		rgb = 0.0;
	}
	al_clear_to_color(dot_darken_color(DOT_GAME_BG_COLOR, rgb));
	al_hold_bitmap_drawing(true);
	al_draw_bitmap(app->bitmap[DOT_BITMAP_BG], 0, 0, 0);
	if(app->game.combo)
	{
		s = app->game.player.ball.r * 2.0 + 128.0 - c * 128.0;
		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_COMBO], al_map_rgba_f(0.125, 0.125, 0.125, 0.125), app->game.player.ball.x - s / 2.0, app->game.player.ball.y - s / 2.0, app->game.player.ball.z, s, s, 0);
	}
	if(app->game.shield.active)
	{
		s = app->game.shield.r * 2.0;
		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_COMBO], al_map_rgba_f(0.125, 0.125, 0.0625, 0.125), app->game.shield.x - s / 2.0, app->game.shield.y - s / 2.0, app->game.player.ball.z, s, s, 0);
	}
	for(i = 0; i < DOT_GAME_MAX_BALLS; i++)
	{
		if(app->game.ball[i].active)
		{
			t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.ball[i].type], t3f_color_white, app->game.ball[i].x - app->game.ball[i].r, app->game.ball[i].y - app->game.ball[i].r, app->game.ball[i].z, app->game.ball[i].r * 2.0, app->game.ball[i].r * 2.0, 0);
		}
	}
	dot_create_touch_dots_effect(data);
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_SCRATCH], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 0, t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT, 0.0, DOT_GAME_PLAYFIELD_WIDTH, al_get_bitmap_height(app->bitmap[DOT_BITMAP_SCRATCH]), 0);
	if(app->game.player.ball.active)
	{
		if(app->game.state == DOT_GAME_STATE_START)
		{
			player_color = al_map_rgba_f(1.0 * 0.5, 1.0 * 0.5, 1.0 * 0.5, 0.5);
		}
		cx = (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2);
		cy = (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2);
		ecx = (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_EYES]) / 2);
		ecy = (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_EYES]) / 2);
		t3f_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], player_color, cx, cy, app->game.player.ball.x, app->game.player.ball.y, app->game.player.ball.z, 0.0, app->game.player.ball.r / cx, app->game.player.ball.r / cy, 0);
		t3f_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_EYES], player_color, 8.0, 8.0, app->game.player.ball.x, app->game.player.ball.y, app->game.player.ball.z, app->game.player.ball.a, app->game.player.ball.r / ecx, app->game.player.ball.r / ecy, 0);
		t3f_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], al_map_rgba_f(0.5, 0.5, 0.5, 0.5), cx, cy, app->game.player.ball.x, 960 - DOT_GAME_PLAYFIELD_HEIGHT + app->game.player.ball.y, app->game.player.ball.z, 0.0, app->game.player.ball.r / cx, app->game.player.ball.r / cy, 0);
		if(app->game.combo)
		{
			sprintf(buf, "%d", dot_game_get_combo_score(data));
			dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), app->game.player.ball.x, app->game.player.ball.y - app->game.player.ball.r - 16.0 - 8.0, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_CENTRE, buf);
		}
	}
	dot_game_render_hud(data);
	if((app->game.tick / 6) % 2)
	{
		text_color = al_map_rgba_f(1.0, 1.0, 0.0, 1.0);
	}
	al_hold_bitmap_drawing(false);
	if(app->game.state == DOT_GAME_STATE_PAUSE)
	{
		al_draw_filled_rectangle(0.0, 0.0, t3f_virtual_display_width, t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		s = DOT_GAME_GRAB_SPOT_SIZE;
		al_hold_bitmap_drawing(true);
		dot_create_grab_spot_effect(data);
		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_SCRATCH], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 0, t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT, 0.0, DOT_GAME_PLAYFIELD_WIDTH, al_get_bitmap_height(app->bitmap[DOT_BITMAP_SCRATCH]), 0);
		dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2 - al_get_font_line_height(app->font[DOT_FONT_32]) / 2, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_CENTRE, "Paused");
//		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_COMBO], al_map_rgba_f(0.125, 0.125, 0.0625, 0.125), app->game.player.ball.x - DOT_GAME_GRAB_SPOT_SIZE, 960 - DOT_GAME_PLAYFIELD_HEIGHT + app->game.player.ball.y - DOT_GAME_GRAB_SPOT_SIZE, 0.0, s * 2, s * 2, 0);
//		al_draw_filled_circle(app->game.player.ball.x, 960 - DOT_GAME_PLAYFIELD_HEIGHT + app->game.player.ball.y, DOT_GAME_GRAB_SPOT_SIZE, al_map_rgba_f(0.5, 0.5, 0.5, 0.5));
	}
	else if(app->game.state == DOT_GAME_STATE_START)
	{
		al_draw_filled_rectangle(0.0, 0.0, t3f_virtual_display_width, t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		al_hold_bitmap_drawing(true);
		dot_create_touch_start_effect(data);
		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_SCRATCH], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 0, t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT, 0.0, DOT_GAME_PLAYFIELD_WIDTH, al_get_bitmap_height(app->bitmap[DOT_BITMAP_SCRATCH]), 0);
		if(app->game.level_start)
		{
			sprintf(buf, "Level %d", app->game.level + 1);
			dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2 - al_get_font_line_height(app->font[DOT_FONT_32]) / 2, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, ALLEGRO_ALIGN_CENTRE, buf);
		}
		dot_shadow_text(app->font[DOT_FONT_16], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT / 2 - al_get_font_line_height(app->font[DOT_FONT_16]) / 2, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_CENTRE, "Touch Here");
	}
	al_hold_bitmap_drawing(false);
}
