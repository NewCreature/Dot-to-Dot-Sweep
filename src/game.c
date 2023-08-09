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
#include "bg_object.h"
#include "intro.h"
#include "leaderboard.h"

static void dot_game_target_balls(void * data, int type)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	for(i = 0; i < app->game.ball_count; i++)
	{
		if(app->game.ball[i].type == type)
		{
			app->game.ball[i].target_tick = DOT_GAME_TARGET_TICKS;
		}
	}
}

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
	app->game.a_bob_and_weave_ticks = 0;

	/* clear the area where the player is so we don't get cheap deaths */
	for(i = 0; i < app->game.ball_count; i++)
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
	app->game.ball_count = 0;
	for(i = 0; i < num_balls && i < DOT_GAME_MAX_BALLS; i++)
	{
		app->game.ball[i].r = DOT_GAME_BALL_SIZE;
		app->game.ball[i].x = t3f_drand(&app->rng_state) * ((float)(DOT_GAME_PLAYFIELD_WIDTH) - app->game.ball[i].r * 2.0) + app->game.ball[i].r;
		app->game.ball[i].y = t3f_drand(&app->rng_state) * ((float)(DOT_GAME_PLAYFIELD_HEIGHT) - app->game.ball[i].r * 2.0) + app->game.ball[i].r;
		app->game.ball[i].z = 0;
		app->game.ball[i].a = t3f_drand(&app->rng_state) * ALLEGRO_PI * 2.0;
		app->game.ball[i].s = t3f_drand(&app->rng_state) * 0.75 + 0.25;
		app->game.ball[i].s *= app->game.speed_multiplier;
		app->game.ball[i].vx = cos(app->game.ball[i].a) * app->game.ball[i].s;
		app->game.ball[i].vy = sin(app->game.ball[i].a) * app->game.ball[i].s;
		app->game.ball[i].type = col;
		col++;
		if(col > 5)
		{
			col = 0;
		}
		app->game.ball[i].active = true;
		app->game.ball_count++;
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
		app->game.ball[j].s *= app->game.speed_multiplier;
		app->game.ball[j].vx = cos(app->game.ball[j].a) * app->game.ball[j].s;
		app->game.ball[j].vy = sin(app->game.ball[j].a) * app->game.ball[j].s;
		app->game.ball[j].type = 6;
		app->game.ball[j].active = true;
		app->game.ball_count++;
	}

	/* drop the player with a random color */
	dot_game_drop_player(data, t3f_rand(&app->rng_state) % 6);
	t3f_play_sample(app->sample[DOT_SAMPLE_START], 1.0, 0.0, 1.0);
	app->game.player.ball.r = 16.0;
	dot_game_target_balls(data, app->game.player.ball.type);
	app->game.a_combo_broken = false;

	app->game.level = level;
	app->game.speed = DOT_GAME_LEVEL_BASE_SPEED;
	app->game.speed_inc = DOT_GAME_LEVEL_TOP_SPEED / (float)num_balls;
	app->game.level_start = true;
}

static void compute_bg_color(APP_INSTANCE * app)
{
	float rgb = 1.0;
	int i, m;

	/* make level colors darker after every 10 levels */
	m = app->game.level / 10;
	for(i = 0; i < m; i++)
	{
		rgb *= 0.75;
	}
	app->game.bg_color = dot_darken_color(dot_transition_color(app->game.old_bg_color, app->level_color[app->game.level % 10], app->game.bg_color_fade), rgb);
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
	dot_game_setup_level(data, app->game.start_level);
	app->game.score = 0;
	app->game.combo = 0;
	app->game.lives = app->game.start_lives;
	app->game.shield.active = false;
	app->game.old_bg_color = app->level_color[0];
	app->game.bg_color_fade = 0.0;
	if(app->music_enabled)
	{
		t3f_play_music(DOT_MUSIC_BGM);
	}
	app->game.tick = 0;
	app->game.block_click = false;
	compute_bg_color(app);
	app->state = DOT_STATE_GAME;
}

int dot_get_leaderboard_spot(T3NET_LEADERBOARD * lp, const char * name, unsigned long score)
{
	int i;

	for(i = 0; i < lp->entries; i++)
	{
		if(!strcmp(lp->entry[i]-> name, name) && lp->entry[i]->score == score)
		{
			return i;
		}
	}
	return -1;
}

/* finish the game */
void dot_game_exit(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	char buf[256] = {0};

	/* save high score */
	sprintf(buf, "%d", app->game.high_score);
	al_set_config_value(t3f_user_data, "Game Data", "High Score", buf);
	t3f_save_user_data();

	/* upload score */
	if(app->upload_scores && !app->demo_file && !app->game.cheats_enabled)
	{
		sprintf(buf, "%d", app->game.level + 1);
		al_stop_timer(t3f_timer);
		dot_show_message(data, "Uploading score...");
		if(t3net_upload_score(app->leaderboard_submit_url, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "normal", "none", app->user_key, dot_leaderboard_obfuscate_score(app->game.score), buf))
		{
			dot_show_message(data, "Downloading leaderboard...");
			app->leaderboard = t3net_get_leaderboard(app->leaderboard_retrieve_url, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "normal", "none", 10, 0);
			if(app->leaderboard)
			{
				app->leaderboard_spot = dot_get_leaderboard_spot(app->leaderboard, app->user_name, dot_leaderboard_obfuscate_score(app->game.score));
			}
		}
		al_resume_timer(t3f_timer);
	}

	/* go back to intro */
	dot_intro_setup(data);
	if(!app->leaderboard)
	{
		app->state = DOT_STATE_INTRO;
		if(app->music_enabled)
		{
			t3f_play_music(DOT_MUSIC_TITLE);
		}
	}
	else
	{
		if(app->music_enabled)
		{
			t3f_stop_music();
		}
		app->state = DOT_STATE_LEADERBOARD;
		app->current_menu = DOT_MENU_LEADERBOARD_2;
	}
}

static int dot_game_get_combo_score(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	return DOT_GAME_BASE_POINTS * app->game.combo + DOT_GAME_COMBO_POINTS * (app->game.combo - 1);
}

static float dot_spread_effect_particle(int pos, int max, float size)
{
	return ((float)(pos + 1) / (float)max) * size - size / 2.0;
}

static void dot_game_create_score_effect(void * data, float x, float y, int number)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char buf[16] = {0};
	char cbuf[2] = {0};
	int i, j;
	float ox, px;
	int ni;
	float w, cw;

	sprintf(buf, "%d", number);
	w = t3f_get_text_width(app->font[DOT_FONT_16], buf);
	ox = w / 2;
	px = 0.0;
	for(i = 0; i < strlen(buf); i++)
	{
		ni = buf[i] - '0';
		cbuf[0] = buf[i];
		cw = t3f_get_text_width(app->font[DOT_FONT_16], cbuf);
		for(j = 0; j < app->number_particle_list[ni].items; j++)
		{
			dot_create_particle(&app->particle[app->current_particle], x + (float)app->number_particle_list[ni].item[j].x + px - ox, y + app->number_particle_list[ni].item[j].y, 0.0, dot_spread_effect_particle(app->number_particle_list[ni].item[j].x + px, w, strlen(buf) * 2.5), dot_spread_effect_particle(app->number_particle_list[ni].item[j].y, t3f_get_font_line_height(app->font[DOT_FONT_16]), 4.0), -10.0, 0.0, 3.0, 45, app->bitmap[DOT_BITMAP_PARTICLE], t3f_color_white);
			app->current_particle++;
			if(app->current_particle >= DOT_MAX_PARTICLES)
			{
				app->current_particle = 0;
			}
		}
		px += cw;
	}
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
		gy = sin(ga) * t3f_drand(&app->rng_state) * r * t3f_drand(&app->rng_state);
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

	if(app->game.ascore > 0)
	{
		app->game.score += app->game.ascore;
		if(app->game.score > app->game.high_score)
		{
			app->game.high_score = app->game.score;
		}
		dot_game_create_score_effect(data, app->game.player.ball.x, app->game.player.ball.y - app->game.player.ball.r - 16.0 - 8.0, app->game.ascore);
		app->game.ascore = 0;
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

static bool balls_collide(DOT_BALL * b1, DOT_BALL * b2)
{
	if(b1->active && b2->active)
	{
		if(fabs(b1->x - b2->x) < b1->r + b2->r && fabs(b1->y - b2->y) < b1->r + b2->r)
		{
			if(t3f_distance(b1->x, b1->y, b2->x, b2->y) < b1->r + b2->r)
			{
				return true;
			}
		}
	}
	return false;
}

/* return the number of colored balls left */
int dot_game_move_balls(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int colored = 0;
	int i, j;

	/* move the balls */
	for(i = 0; i < app->game.ball_count; i++)
	{
		if(app->game.ball[i].active)
		{
			dot_game_move_ball(data, i);
			if(app->game.ball[i].type != DOT_BITMAP_BALL_BLACK)
			{
				colored++;
			}
			if(app->game.ball[i].target_tick > 0)
			{
				app->game.ball[i].target_tick--;
			}
		}
	}

	/* mark obscured balls */
	for(i = 0; i < app->game.ball_count; i++)
	{
		app->game.ball[i].obscured = false;
		for(j = 0; j < app->game.ball_count; j++)
		{
			if(i != j && app->game.ball[i].type != app->game.ball[j].type && balls_collide(&app->game.ball[i], &app->game.ball[j]))
			{
				app->game.ball[i].obscured = true;
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

static float get_angle_diff(float a1, float a2)
{
	if(a1 < -ALLEGRO_PI)
	{
		a1 += ALLEGRO_PI * 2.0;
	}
	else if(a1 > ALLEGRO_PI)
	{
		a1 -= ALLEGRO_PI * 2.0;
	}
	if(a2 < -ALLEGRO_PI)
	{
		a2 += ALLEGRO_PI * 2.0;
	}
	else if(a2 > ALLEGRO_PI)
	{
		a2 -= ALLEGRO_PI * 2.0;
	}
	return fabs(a1 - a2);
}

void dot_game_check_player_collisions(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i, j;

	/* see if player ball hits any other balls */
	for(i = 0; i < app->game.ball_count; i++)
	{
		if(app->game.ball[i].active)
		{
			if(balls_collide(&app->game.ball[i], &app->game.player.ball))
			{
				app->game.a_bob_and_weave_ticks = 0;
				/* hitting the same color gives you points and increases your combo */
				if(app->game.ball[i].type == app->game.player.ball.type)
				{
					t3f_play_sample(app->sample[DOT_SAMPLE_GRAB], 1.0, 0.0, 1.0);
					app->game.ball[i].active = false;
					if(app->game.player.ball.timer < DOT_GAME_COMBO_TIME)
					{
						app->game.combo++;
					}
					app->game.ascore = dot_game_get_combo_score(data);
					app->game.player.ball.timer = 0;
					for(j = 0; j < app->game.ball_count; j++)
					{
						if(app->game.ball[j].active && app->game.ball[j].type != DOT_BITMAP_BALL_BLACK && app->game.ball[j].type != app->game.player.ball.type)
						{
							app->game.ball[j].type = app->game.player.ball.type;
							app->game.ball[j].target_tick = DOT_GAME_TARGET_TICKS;
							break;
						}
					}
					app->game.speed += app->game.speed_inc;
					dot_game_create_splash_effect(data, app->game.ball[i].x, app->game.ball[i].y, app->game.ball[i].r, app->dot_color[app->game.ball[i].type]);
				}

				/* hitting other color kills you */
				else
				{
					if(app->touch_id >= 0)
					{
						t3f_touch[app->touch_id].active = false;
					}
					dot_game_create_splash_effect(data, app->game.player.ball.x, app->game.player.ball.y, app->game.player.ball.r, app->dot_color[app->game.player.ball.type]);
					t3f_play_sample(app->sample[DOT_SAMPLE_LOSE], 1.0, 0.0, 1.0);
					dot_game_accumulate_score(data);
					app->game.combo = 0;
					app->game.shield.active = false;
					if(app->game.lives > 0)
					{
						app->game.lives--;
					}
					app->game.emo_tick = 60;
					app->game.emo_state = DOT_GAME_EMO_STATE_DEAD;
					app->game.a_combo_broken = true;

					/* change ball color to match the ball that is hit unless it is black */
					if(app->game.ball[i].type != DOT_BITMAP_BALL_BLACK)
					{
						if(app->game.lives > 0 || app->game.start_lives <= 0)
						{
							dot_game_drop_player(data, app->game.ball[i].type);
							dot_game_target_balls(data, app->game.player.ball.type);
						}
						else
						{
							app->game.player.ball.active = false;
							app->game.state = DOT_GAME_STATE_DONE;
						}
					}
					else
					{
						if(app->game.lives > 0 || app->game.start_lives <= 0)
						{
							dot_game_drop_player(data, app->game.player.ball.type);
						}
						else
						{
							app->game.player.ball.active = false;
							app->game.state = DOT_GAME_STATE_DONE;
						}
					}
					al_show_mouse_cursor(t3f_display);
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

	float ox, oy, angle_dir, angle_div, angle_diff;

	if(app->game.player.ball.active)
	{
		ox = app->game.player.ball.x;
		oy = app->game.player.ball.y;

		if(app->using_controller)
		{
			app->game.player.ball.x += app->controller.axis_x * 4.0;
			app->game.player.ball.y += app->controller.axis_y * 4.0;
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
				al_show_mouse_cursor(t3f_display);
			}
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
			angle_div = 24.0 - t3f_distance(ox, oy, app->game.player.ball.x, app->game.player.ball.y);
			if(angle_div < 1.0)
			{
				angle_div = 1.0;
			}
			app->game.player.old_target_angle = app->game.player.target_angle;
			app->game.player.target_angle = atan2(oy - app->game.player.ball.y, ox - app->game.player.ball.x);
			angle_dir = get_angle_dir(app->game.player.target_angle, app->game.player.ball.a);
			angle_diff = get_angle_diff(app->game.player.ball.a, app->game.player.target_angle);
			if(angle_diff < ALLEGRO_PI / 4.0 && angle_div > 20.0)
			{
				angle_div = angle_div * (angle_div / 5.0);
			}
			if(angle_dir < 0)
			{
				if(angle_diff <= ALLEGRO_PI / angle_div)
				{
					app->game.player.ball.a = app->game.player.target_angle;
				}
				else
				{
					app->game.player.ball.a -= ALLEGRO_PI / angle_div;
				}
			}
			else
			{
				if(angle_diff <= ALLEGRO_PI / angle_div)
				{
					app->game.player.ball.a = app->game.player.target_angle;
				}
				else
				{
					app->game.player.ball.a += ALLEGRO_PI / angle_div;
				}
			}
			if(app->game.player.ball.a > ALLEGRO_PI)
			{
				app->game.player.ball.a -= ALLEGRO_PI * 2.0;
			}
			if(app->game.player.ball.a < -ALLEGRO_PI)
			{
				app->game.player.ball.a += ALLEGRO_PI * 2.0;
			}
		}

		/* handle combo timer */
		if(app->game.combo > 0)
		{
			app->game.player.ball.timer++;
			if(app->game.player.ball.timer >= DOT_GAME_COMBO_TIME)
			{
				if(app->game.combo >= 10)
				{
					app->game.emo_tick = 60;
					app->game.emo_state = DOT_GAME_EMO_STATE_WOAH;
				}
				dot_game_accumulate_score(data);
				app->game.player.ball.timer = 0;
				app->game.combo = 0;
				app->game.shield.active = false;
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

static int dot_game_get_emo_blink_time(T3F_RNG_STATE * rp)
{
	return 180 + t3f_random(rp, 60);
}

void dot_game_emo_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(app->game.emo_state)
	{
		case DOT_GAME_EMO_STATE_NORMAL:
		{
			app->game.emo_tick--;
			if(app->game.emo_tick <= 0)
			{
				app->game.emo_tick = 3;
				app->game.emo_state = DOT_GAME_EMO_STATE_BLINK;
			}
			break;
		}
		case DOT_GAME_EMO_STATE_BLINK:
		{
			app->game.emo_tick--;
			if(app->game.emo_tick <= 0)
			{
				app->game.emo_tick = dot_game_get_emo_blink_time(&app->rng_state);
				app->game.emo_state = DOT_GAME_EMO_STATE_NORMAL;
			}
			break;
		}
		case DOT_GAME_EMO_STATE_WOAH:
		{
			app->game.emo_tick--;
			if(app->game.emo_tick <= 0)
			{
				app->game.emo_tick = dot_game_get_emo_blink_time(&app->rng_state);
				app->game.emo_state = DOT_GAME_EMO_STATE_NORMAL;
			}
			break;
		}
		case DOT_GAME_EMO_STATE_DEAD:
		{
			app->game.emo_tick--;
			if(app->game.emo_tick <= 0)
			{
				app->game.emo_tick = dot_game_get_emo_blink_time(&app->rng_state);
				app->game.emo_state = DOT_GAME_EMO_STATE_NORMAL;
			}
			break;
		}
	}
}

static void exit_to_title(APP_INSTANCE * app)
{
	dot_intro_setup(app);
	al_show_mouse_cursor(t3f_display);
	app->state = DOT_STATE_INTRO;
	if(app->music_enabled)
	{
		t3f_play_music(DOT_MUSIC_TITLE);
	}
}

/* the main game logic function */
void dot_game_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int colored = 0;
	int i;

	/* handle level bg color transition */
	if(app->game.bg_color_fade < 1.0)
	{
		app->game.bg_color_fade += 1.0 / 60.0;
		if(app->game.bg_color_fade > 1.0)
		{
			app->game.bg_color_fade = 1.0;
		}
	}

	compute_bg_color(app);

	dot_game_emo_logic(data);
	dot_bg_objects_logic(data, app->game.speed);
	switch(app->game.state)
	{

		/* balls move slow for a few seconds so player can get ready */
		case DOT_GAME_STATE_START:
		{
			app->game.state_tick++;
			if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
			{
				exit_to_title(app);
				t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
				t3f_key[ALLEGRO_KEY_BACK] = 0;
			}
			else
			{
				if(app->touch_id >= 0)
				{
					if(!app->game.block_click && app->touch_x >= DOT_GAME_TOUCH_START_X && app->touch_x < DOT_GAME_TOUCH_END_X && app->touch_y >= DOT_GAME_TOUCH_START_Y && app->touch_y < DOT_GAME_TOUCH_END_Y)
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
						al_hide_mouse_cursor(t3f_display);
						app->game.block_click = true;
					}
				}
				else if(app->controller.button)
				{
					if(!app->game.block_click)
					{
						t3f_play_sample(app->sample[DOT_SAMPLE_GO], 1.0, 0.0, 1.0);
						app->game.state = DOT_GAME_STATE_PLAY;
						app->game.state_tick = 0;
						app->game.player.lost_touch = false;
						app->game.player.ball.active = true;
						app->game.player.want_shield = true;
						app->game.player.touch_offset_x = 0;
						app->game.player.touch_offset_y = 0;
						app->game.player.ball.x = DOT_GAME_PLAYFIELD_WIDTH / 2;
						app->game.player.ball.y = DOT_GAME_PLAYFIELD_HEIGHT / 2;
						app->game.level_start = false;
						al_hide_mouse_cursor(t3f_display);
						app->game.block_click = true;
					}
					app->controller.button = false;
				}
				else
				{
					app->game.block_click = false;
				}

				/* handle ball logic */
				colored = dot_game_move_balls(data);
			}
			break;
		}

		case DOT_GAME_STATE_PAUSE:
		{
			if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
			{
				exit_to_title(app);
				t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
				t3f_key[ALLEGRO_KEY_BACK] = 0;
			}
			else
			{
				if(app->touch_id >= 0)
				{
					if(!app->game.block_click && t3f_distance(app->touch_x, app->touch_y, app->game.player.ball.x, app->game.player.ball.y) < DOT_GAME_GRAB_SPOT_SIZE)
					{
						app->game.player.touch_offset_x = app->game.player.ball.x - app->touch_x;
						app->game.player.touch_offset_y = app->game.player.ball.y - app->touch_y;
						app->game.state = DOT_GAME_STATE_PLAY;
						al_hide_mouse_cursor(t3f_display);
						app->game.block_click = true;
					}
				}
				else if(app->controller.button)
				{
					if(!app->game.block_click)
					{
						app->game.state = DOT_GAME_STATE_PLAY;
						app->controller.button = false;
						al_hide_mouse_cursor(t3f_display);
					}
				}
				else
				{
					app->game.block_click = false;
				}
			}
			break;
		}

		case DOT_GAME_STATE_PAUSE_MENU:
		{
			ALLEGRO_COLOR text_color;

			if((app->game.tick / 6) % 2)
			{
				text_color = al_map_rgba_f(1.0, 1.0, 0.0, 1.0);
			}
			else
			{
				text_color = t3f_color_white;
			}
			app->menu[DOT_MENU_PAUSE]->element[0].color = text_color;
			if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
			{
				app->game.state = app->game.pause_state;
				t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
				t3f_key[ALLEGRO_KEY_BACK] = 0;
			}
			else
			{
				if(app->controller.axis_y_pressed)
				{
					if(app->controller.axis_y < 0.0)
					{
						t3f_select_previous_gui_element(app->menu[DOT_MENU_PAUSE]);
						app->controller.axis_y_pressed = false;
					}
					else
					{
						t3f_select_next_gui_element(app->menu[DOT_MENU_PAUSE]);
						app->controller.axis_y_pressed = false;
					}
				}
				if(app->controller.button)
				{
					t3f_activate_selected_gui_element(app->menu[DOT_MENU_PAUSE], app);
					app->controller.button = false;
				}
			}
			t3f_process_gui(app->menu[DOT_MENU_PAUSE], app);
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
			if(t3f_key[ALLEGRO_KEY_ESCAPE] || app->controller.current_joy_disconnected)
			{
				app->game.pause_state = app->game.state;
				app->game.state = DOT_GAME_STATE_PAUSE;
				al_show_mouse_cursor(t3f_display);
				t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
				app->controller.button = false;
			}
			else if(app->controller.button)
			{
				app->game.pause_state = app->game.state;
				app->game.state = DOT_GAME_STATE_PAUSE_MENU;
				app->menu[DOT_MENU_PAUSE]->hover_element = -1;
				t3f_select_next_gui_element(app->menu[DOT_MENU_PAUSE]);
				app->controller.axis_y_pressed = false;
				al_show_mouse_cursor(t3f_display);
				t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
				app->controller.button = false;
			}
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
				t3f_update_achievement_progress(app->achievements, DOT_ACHIEVEMENT_GETTING_INTO_IT, 1);
				if(!app->game.a_combo_broken)
				{
					t3f_update_achievement_progress(app->achievements, DOT_ACHIEVEMENT_FULL_COMBO, 1);
				}
				if(app->game.combo >= 10)
				{
					app->game.emo_tick = 60;
					app->game.emo_state = DOT_GAME_EMO_STATE_WOAH;
				}
				dot_game_accumulate_score(data);
				for(i = 0; i < app->game.ball_count; i++)
				{
					if(app->game.ball[i].active && app->game.ball[i].type == 6)
					{
						dot_game_create_splash_effect(data, app->game.ball[i].x, app->game.ball[i].y, app->game.ball[i].r, app->dot_color[app->game.ball[i].type]);
					}
				}
				if(app->touch_id >= 0)
				{
					t3f_touch[app->touch_id].active = false;
				}
				al_show_mouse_cursor(t3f_display);
				app->game.old_bg_color = app->game.bg_color;
				dot_game_setup_level(data, app->game.level + 1);
				app->game.bg_color_fade = 0.0;
				app->game.combo = 0;
				app->game.shield.active = false;
			}
			break;
		}
	}
	app->controller.current_joy_disconnected = false;
	app->game.a_bob_and_weave_ticks++;
	if(app->game.a_bob_and_weave_ticks >= 300)
	{
		t3f_update_achievement_progress(app->achievements, DOT_ACHIEVEMENT_BOB_AND_WEAVE, 1);
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
	if(app->desktop_mode)
	{
		if(app->state == DOT_STATE_GAME && (app->game.state == DOT_GAME_STATE_PAUSE || app->game.state == DOT_GAME_STATE_START))
		{
			al_draw_filled_rectangle(0, DOT_GAME_PLAYFIELD_HEIGHT, 540, DOT_GAME_PLAYFIELD_HEIGHT + 80, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		}
	}
	al_hold_bitmap_drawing(true);
	sprintf(buffer, "Score");
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, t3f_virtual_display_width - 8, 440 + 40 - t3f_get_font_line_height(app->font[DOT_FONT_32]), DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_RIGHT, buffer);
	sprintf(buffer, "%d", app->game.score);
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, t3f_virtual_display_width - 8, 440 + 40, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_RIGHT, buffer);
	sprintf(buffer, "Lives");
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, 8, 440 + 40 - t3f_get_font_line_height(app->font[DOT_FONT_32]), DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, 0, buffer);
	sprintf(buffer, "%d", app->game.lives);
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, 8, 440 + 40, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, 0, buffer);

	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_EMO_BG], shadow, t3f_virtual_display_width / 2 - 32 + DOT_SHADOW_OX * 2, DOT_GAME_PLAYFIELD_HEIGHT + 40 - 32 + DOT_SHADOW_OY * 2, 0, 64, 64, 0);
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_EMO_BG], app->dot_color[app->game.player.ball.type], t3f_virtual_display_width / 2 - 32, DOT_GAME_PLAYFIELD_HEIGHT + 40 - 32, 0, 64, 64, 0);
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_EMO_NORMAL + app->game.emo_state], t3f_color_white, t3f_virtual_display_width / 2 - 32, DOT_GAME_PLAYFIELD_HEIGHT + 40 - 32, 0.0, 64, 64, 0);
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_EMO_FG], t3f_color_white, t3f_virtual_display_width / 2 - 32, DOT_GAME_PLAYFIELD_HEIGHT + 40 - 32, 0, 64, 64, 0);
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
	al_set_clipping_rectangle(0, 0, 512, 512);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 1.0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
	s = DOT_GAME_GRAB_SPOT_SIZE;
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_COMBO], al_map_rgba_f(0.0, 0.0, 0.0, 1.0), (float)(app->game.player.ball.x - DOT_GAME_GRAB_SPOT_SIZE) * sx, app->game.player.ball.y - DOT_GAME_GRAB_SPOT_SIZE, 0.0, (s * 2.0) * sx, s * 2, 0);
	al_restore_state(&old_state);
	al_hold_bitmap_drawing(held);
	t3f_set_clipping_rectangle(0, 0, 0, 0);
}

static void dot_create_touch_dots_effect(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
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
	al_set_clipping_rectangle(0, 0, 512, 512);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
	al_hold_bitmap_drawing(true);
	for(i = 0; i < app->game.ball_count; i++)
	{
		if(app->game.ball[i].active)
		{
			t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.ball[i].type], al_map_rgba_f(1.0, 1.0, 1.0, 1.0), (app->game.ball[i].x - app->game.ball[i].r) * sx, app->game.ball[i].y - app->game.ball[i].r, app->game.ball[i].z, (app->game.ball[i].r * 2.0) * sx, app->game.ball[i].r * 2.0, 0);
		}
	}
	al_hold_bitmap_drawing(false);
	al_restore_state(&old_state);
	al_hold_bitmap_drawing(held);
	t3f_set_clipping_rectangle(0.0, 0.0, 0.0, 0.0);
}

static void dot_create_touch_start_effect(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
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
	al_set_clipping_rectangle(0, 0, 512, 512);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 1.0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
	al_draw_filled_rectangle(DOT_GAME_TOUCH_START_X * sx, DOT_GAME_TOUCH_START_Y, DOT_GAME_TOUCH_END_X * sx, DOT_GAME_TOUCH_END_Y, al_map_rgba_f(1.0, 1.0, 1.0, 1.0));
	al_restore_state(&old_state);
	al_hold_bitmap_drawing(held);
	t3f_set_clipping_rectangle(0, 0, 0, 0);
}

/* main game render function */
void dot_game_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char buf[16] = {0};

	int i;
	ALLEGRO_COLOR text_color = t3f_color_white;
	float c = (float)app->game.player.ball.timer / (float)DOT_GAME_COMBO_TIME;
	float s, r, a;
	float cx, cy, ecx, ecy;
	float touch_effect_y = 0;
	float level_y = 8;
	float start_y = DOT_GAME_PLAYFIELD_HEIGHT / 2;
	char * touch_text = "Click";

	if(!app->desktop_mode)
	{
		touch_effect_y = t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT;
		level_y = DOT_GAME_PLAYFIELD_HEIGHT / 2 - t3f_get_font_line_height(app->font[DOT_FONT_32]) / 2;
		start_y = t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT / 2;
		touch_text = "Touch";
	}
	al_clear_to_color(app->game.bg_color);
	al_hold_bitmap_drawing(true);
	dot_bg_objects_render(data);
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
	for(i = 0; i < app->game.ball_count; i++)
	{
		if(app->game.ball[i].active)
		{
			if(app->game.ball[i].obscured)
			{
				a = 0.75;
			}
			else
			{
				a = 1.0;
			}
			t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.ball[i].type], al_map_rgba_f(a, a, a, a), app->game.ball[i].x - app->game.ball[i].r, app->game.ball[i].y - app->game.ball[i].r, app->game.ball[i].z, app->game.ball[i].r * 2.0, app->game.ball[i].r * 2.0, 0);
		}
	}
	for(i = 0; i < app->game.ball_count; i++)
	{
		if(app->game.ball[i].active)
		{
			if(app->game.ball[i].type == app->game.player.ball.type)
			{
				r = 32.0 + app->game.ball[i].target_tick * 2;
				a = 1.0 - app->game.ball[i].target_tick / (float)DOT_GAME_TARGET_TICKS;
				t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_TARGET], app->game.ball[i].obscured ? al_map_rgba_f(a, a, 0.5 * a, a) : al_map_rgba_f(a, a, a, a), app->game.ball[i].x - r, app->game.ball[i].y - r, app->game.ball[i].z, r * 2.0, r * 2.0, 0);
			}
		}
	}
	if(!app->desktop_mode)
	{
		dot_create_touch_dots_effect(data);
		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_SCRATCH], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 0, t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT, 0.0, DOT_GAME_PLAYFIELD_WIDTH, al_get_bitmap_height(app->bitmap[DOT_BITMAP_SCRATCH]), 0);
	}
	if(app->game.player.ball.active)
	{
		cx = (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2);
		cy = (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]) / 2);
		ecx = (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_EYES]) / 2);
		ecy = (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_EYES]) / 2);
		if(app->game.state != DOT_GAME_STATE_START)
		{
			t3f_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type], t3f_color_white, cx, cy, app->game.player.ball.x, app->game.player.ball.y, app->game.player.ball.z, 0.0, app->game.player.ball.r / cx, app->game.player.ball.r / cy, 0);
			t3f_draw_scaled_rotated_bitmap(app->bitmap[DOT_BITMAP_BALL_EYES], t3f_color_white, ecx, ecy, app->game.player.ball.x, app->game.player.ball.y, app->game.player.ball.z, app->game.player.ball.a, app->game.player.ball.r / ecx, app->game.player.ball.r / ecy, 0);
		}
		if(app->game.combo)
		{
			sprintf(buf, "%d", app->game.ascore);
			dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), app->game.player.ball.x, app->game.player.ball.y - app->game.player.ball.r - 16.0 - 8.0, DOT_SHADOW_OX, DOT_SHADOW_OY, T3F_FONT_ALIGN_CENTER, buf);
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
		if(!app->desktop_mode)
		{
			al_draw_filled_rectangle(0.0, 0.0, t3f_virtual_display_width, t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		}
		al_hold_bitmap_drawing(true);
		s = DOT_GAME_GRAB_SPOT_SIZE;
		dot_create_grab_spot_effect(data);
		if(app->desktop_mode)
		{
			t3f_set_clipping_rectangle(0, 0, DOT_GAME_PLAYFIELD_WIDTH, DOT_GAME_PLAYFIELD_HEIGHT);
		}
		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_SCRATCH], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 0, touch_effect_y, 0.0, DOT_GAME_PLAYFIELD_WIDTH, al_get_bitmap_height(app->bitmap[DOT_BITMAP_SCRATCH]), 0);
		if(app->desktop_mode)
		{
			al_hold_bitmap_drawing(false);
			al_hold_bitmap_drawing(true);
			t3f_set_clipping_rectangle(0, 0, 0, 0);
		}
		dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2 - t3f_get_font_line_height(app->font[DOT_FONT_32]) / 2, DOT_SHADOW_OX, DOT_SHADOW_OY, T3F_FONT_ALIGN_CENTER, "Paused");
	}
	if(app->game.state == DOT_GAME_STATE_PAUSE_MENU)
	{
		al_draw_filled_rectangle(0.0, 0.0, t3f_virtual_display_width, t3f_virtual_display_height, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		al_hold_bitmap_drawing(true);
		t3f_render_gui(app->menu[DOT_MENU_PAUSE]);
	}
	else if(app->game.state == DOT_GAME_STATE_START)
	{
		if(!app->desktop_mode)
		{
			al_draw_filled_rectangle(0.0, 0.0, t3f_virtual_display_width, DOT_GAME_PLAYFIELD_HEIGHT + 80, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		}
		al_hold_bitmap_drawing(true);
		dot_create_touch_start_effect(data);
		if(app->desktop_mode)
		{
			t3f_set_clipping_rectangle(0, 0, DOT_GAME_PLAYFIELD_WIDTH, DOT_GAME_PLAYFIELD_HEIGHT);
		}
		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_SCRATCH], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 0, touch_effect_y, 0.0, DOT_GAME_PLAYFIELD_WIDTH, al_get_bitmap_height(app->bitmap[DOT_BITMAP_SCRATCH]), 0);
		if(app->desktop_mode)
		{
			al_hold_bitmap_drawing(false);
			al_hold_bitmap_drawing(true);
			t3f_set_clipping_rectangle(0, 0, 0, 0);
		}
		if(app->game.level_start)
		{
			sprintf(buf, "Level %d", app->game.level + 1);
			dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, level_y, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, buf);
		}
		dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, start_y - t3f_get_font_line_height(app->font[DOT_FONT_32]), DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, touch_text);
		dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, start_y, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, "Here");
	}
	al_hold_bitmap_drawing(false);
}
