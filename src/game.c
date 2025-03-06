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
#include "mouse.h"

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
	app->game.a_start_lives = app->game.lives;

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
void dot_game_initialize(void * data, bool demo_seed, int mode)
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
	app->game.mode = mode;
	app->game.score = 0;
	app->game.combo = 0;
	if(app->game.start_lives > 0)
	{
		app->game.lives = app->game.start_lives;
	}
	else
	{
		if(mode == 0)
		{
			app->game.lives = 3;
		}
		else
		{
			app->game.lives = 5;
		}
	}
	if(mode == 1)
	{
		app->game.lives_up_threshold = DOT_GAME_EXTRA_LIFE_POINTS;
	}
	else
	{
		app->game.lives_up_threshold = 0;
	}
	app->game.shield.active = false;
	dot_game_setup_level(data, app->game.start_level);
	app->game.old_bg_color = app->level_color[0];
	app->game.bg_color_fade = 0.0;
	if(app->music_enabled)
	{
		t3f_play_music(DOT_MUSIC_BGM);
	}
	app->game.tick = 0;
	app->game.extra_life_tick = 0;
	t3f_clear_touch_state();
	compute_bg_color(app);
	app->state = DOT_STATE_GAME;
}

/* finish the game */
void dot_game_exit(void * data, bool from_menu)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	char buf[256] = {0};
	const char * val;
	bool upload = false;

	/* determine if we need to upload */
	val = al_get_config_value(t3f_user_data, "Game Data", app->game.mode == 0 ? "High Score" : "High Score Easy");
	if(!val || dot_leaderboard_unobfuscate_score(atoi(val)) < app->game.score)
	{
		al_set_config_value(t3f_user_data, "Game Data", app->game.mode == 0 ? "Score Uploaded" : "Easy Score Uploaded", "false");
		upload = true;
	}
	val = al_get_config_value(t3f_user_data, "Game Data", app->game.mode == 0 ? "Score Uploaded" : "Easy Score Uploaded");
	if(val && !strcmp(val, "false"))
	{
		upload = true;
	}

	/* save high score */
	if(app->game.score >= app->game.high_score[app->game.mode])
	{
		sprintf(buf, "%lu", dot_leaderboard_obfuscate_score(app->game.high_score[app->game.mode]));
		al_set_config_value(t3f_user_data, "Game Data", app->game.mode == 0 ? "High Score" : "High Score Easy", buf);
		sprintf(buf, "%d", app->game.level + 1);
		al_set_config_value(t3f_user_data, "Game Data", app->game.mode == 0 ? "High Score Level" : "High Score Level Easy", buf);
		t3f_save_user_data();
	}

	/* upload score */
	if(app->upload_scores && !app->demo_file && !app->game.cheats_enabled)
	{
		al_stop_timer(t3f_timer);
		if(upload)
		{
			dot_show_message(data, "Uploading score...");
			dot_upload_current_high_score(app);
		}
		if(!from_menu)
		{
			dot_show_message(data, "Downloading leaderboard...");
			app->leaderboard = t3net_get_leaderboard(app->leaderboard_retrieve_url, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, app->game.mode == 0 ? "normal" : "easy", "none", 10, 0);
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
		if(!from_menu)
		{
			if(app->music_enabled)
			{
				t3f_stop_music();
			}
			app->current_menu = DOT_MENU_LEADERBOARD_2;
		}
		else
		{
			if(app->music_enabled)
			{
				t3f_play_music(DOT_MUSIC_TITLE);
			}
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
			dot_create_particle(&app->particle[app->current_particle], 0, x + (float)app->number_particle_list[ni].item[j].x + px - ox, y + app->number_particle_list[ni].item[j].y, 0.0, dot_spread_effect_particle(app->number_particle_list[ni].item[j].x + px, w, strlen(buf) * 2.5), dot_spread_effect_particle(app->number_particle_list[ni].item[j].y, t3f_get_font_line_height(app->font[DOT_FONT_16]), 4.0), -10.0, 0.0, 3.0, 45, app->bitmap[DOT_BITMAP_PARTICLE], t3f_color_white);
			app->current_particle++;
			if(app->current_particle >= DOT_MAX_PARTICLES)
			{
				app->current_particle = 0;
			}
		}
		px += cw;
	}
}

static void dot_game_create_extra_life_effect(void * data, float x, float y)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char buf[16] = {0};
	int j;
	float ox, px;
	float w;

	strcpy(buf, DOT_EXTRA_LIFE_TEXT);
	w = t3f_get_text_width(app->font[DOT_FONT_16], buf);
	ox = w / 2;
	px = 0.0;
	for(j = 0; j < app->extra_life_particle_list.items; j++)
	{
		dot_create_particle(&app->particle[app->current_particle], 0, x + (float)app->extra_life_particle_list.item[j].x + px - ox, y + app->extra_life_particle_list.item[j].y, 0.0, dot_spread_effect_particle(app->extra_life_particle_list.item[j].x + px, w, strlen(buf) * 2.5), dot_spread_effect_particle(app->extra_life_particle_list.item[j].y, t3f_get_font_line_height(app->font[DOT_FONT_16]), 4.0), -10.0, 0.0, 3.0, 45, app->bitmap[DOT_BITMAP_PARTICLE], t3f_color_white);
		app->current_particle++;
		if(app->current_particle >= DOT_MAX_PARTICLES)
		{
			app->current_particle = 0;
		}
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
		dot_create_particle(&app->particle[app->current_particle], 1, x + gx, y + gy, 0.0, cos(ga) * t3f_drand(&app->rng_state), sin(ga) * t3f_drand(&app->rng_state), t3f_drand(&app->rng_state) * -5.0 - 5.0, 0.5, 5.0, 30, app->bitmap[DOT_BITMAP_PARTICLE], color);
		app->current_particle++;
		if(app->current_particle >= DOT_MAX_PARTICLES)
		{
			app->current_particle = 0;
		}
	}
}

static void _dot_update_achievement_progress(APP_INSTANCE * app, int id, int progress)
{
	if(!app->game.cheats_enabled)
	{
		t3f_update_achievement_progress(app->achievements, id, progress);
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
		if(app->game.lives_up_threshold > 0 && app->game.score >= app->game.lives_up_threshold)
		{
			t3f_play_sample(app->sample[DOT_SAMPLE_EXTRA_LIFE], 1.0, 0.0, 1.0);
			app->game.lives++;
			app->game.lives_up_threshold += DOT_GAME_EXTRA_LIFE_POINTS;
			dot_game_create_extra_life_effect(data, app->game.player.ball.x, app->game.player.ball.y - app->game.player.ball.r + 16.0 + 8.0);
			app->game.extra_life_tick = 120;
		}
		if(app->game.score > app->game.high_score[app->game.mode])
		{
			app->game.high_score[app->game.mode] = app->game.score;
		}
		if(!t3f_achievement_gotten(app->achievements, DOT_ACHIEVEMENT_GOOD_GAME))
		{
			if(app->game.score >= 100000)
			{
				_dot_update_achievement_progress(app, DOT_ACHIEVEMENT_GOOD_GAME, 1);
			}
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
					if(!t3f_achievement_gotten(app->achievements, DOT_ACHIEVEMENT_OOPS))
					{
						if(app->game.a_oops_ticks <= 60)
						{
							_dot_update_achievement_progress(app, DOT_ACHIEVEMENT_OOPS, 1);
						}
					}
					if(!t3f_achievement_gotten(app->achievements, DOT_ACHIEVEMENT_SO_CLOSE))
					{
						if(app->game.a_colored_balls_remaining <= 1)
						{
							_dot_update_achievement_progress(app, DOT_ACHIEVEMENT_SO_CLOSE, 1);
						}
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
					if(app->input_type == DOT_INPUT_MOUSE)
					{
						t3f_set_mouse_xy(app->game.player.ball.x, app->game.player.ball.y);
					}
					if(app->touch_id >= 0)
					{
						t3f_clear_touch_state();
					}

					/* change ball color to match the ball that is hit unless it is black */
					if(app->game.ball[i].type != DOT_BITMAP_BALL_BLACK)
					{
						if(app->game.lives > 0)
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
					dot_enable_mouse_cursor(true);
					break;
				}
				app->game.a_bob_and_weave_ticks = 0;
			}
		}
	}
}

static void update_player_position_touch(APP_INSTANCE * app)
{
	float scale = 1.0;

	if(app->mouse_sensitivity > 0.0)
	{
		scale = app->mouse_sensitivity;
	}
	app->game.player.ball.x += (app->touch_x - app->old_touch_x) * scale;
	app->game.player.ball.y += (app->touch_y - app->old_touch_y) * scale;
}

static void update_player_position_mouse(APP_INSTANCE * app)
{
	app->game.player.ball.x = app->touch_x;
	app->game.player.ball.y = app->touch_y;
}

static void maybe_activate_shield(APP_INSTANCE * app)
{
	if(app->game.player.want_shield)
	{
		app->game.shield.x = app->game.player.ball.x;
		app->game.shield.y = app->game.player.ball.y;
		app->game.shield.r = app->game.player.ball.r + 1.0;
		app->game.shield.active = true;
		app->game.player.want_shield = false;
	}
}

static void confine_mouse(APP_INSTANCE * app)
{
	float new_x = app->touch_x;
	float new_y = app->touch_y;
	bool update = false;

	/* check left edge */
	if(t3f_get_mouse_x() < app->game.player.ball.r)
	{
		if(t3f_get_mouse_x() > app->game.player.old_mouse_x)
		{
			new_x = app->game.player.ball.x + (t3f_get_mouse_x() - app->game.player.old_mouse_x);
			update = true;
		}
	}

	/* check right edge */
	else if(t3f_get_mouse_x() + app->game.player.ball.r > DOT_GAME_PLAYFIELD_WIDTH + 0.5)
	{
		if(t3f_get_mouse_x() < app->game.player.old_mouse_x)
		{
			new_x = app->game.player.ball.x - (app->game.player.old_mouse_x - t3f_get_mouse_x());
			update = true;
		}
	}

	/* check top edge */
	if(t3f_get_mouse_y() + 0.5 < app->game.player.ball.r)
	{
		if(t3f_get_mouse_y() > app->game.player.old_mouse_y)
		{
			new_y = app->game.player.ball.y + (t3f_get_mouse_y() - app->game.player.old_mouse_y);
			update = true;
		}
	}

	/* check bottom edge */
	else if(t3f_get_mouse_y() + app->game.player.ball.r > DOT_GAME_PLAYFIELD_HEIGHT + 0.5)
	{
		if(t3f_get_mouse_y() < app->game.player.old_mouse_y)
		{
			new_y = app->game.player.ball.y - (app->game.player.old_mouse_y - t3f_get_mouse_y());
			update = true;
		}
	}
	app->game.player.old_mouse_x = t3f_get_mouse_x();
	app->game.player.old_mouse_y = t3f_get_mouse_y();
	if(update)
	{
		t3f_set_mouse_xy(new_x, new_y);
	}
}

static void move_player_with_mouse(APP_INSTANCE * app)
{
	int dx, dy;
	float mouse_sensitivity = 0.0;

	/* mouse sensitivity set by user so use that value */
	if(app->mouse_sensitivity > 0.0)
	{
		mouse_sensitivity = app->mouse_sensitivity;
	}

	/* set 0.667 scale in Steam Deck handheld mode */
	else if(app->on_steam_deck)
	{
		mouse_sensitivity = 0.667;
	}
	if(mouse_sensitivity <= 0.0)
	{
		confine_mouse(app);
		update_player_position_mouse(app);
	}
	else
	{
		t3f_get_mouse_mickeys(&dx, &dy, NULL);
		app->game.player.ball.x += ((float)dx / t3f_current_view->scale_x) * mouse_sensitivity;
		app->game.player.ball.y += ((float)dy / t3f_current_view->scale_y) * mouse_sensitivity;
		#ifndef ALLEGRO_MACOSX
			if(app->game.state == DOT_GAME_STATE_PLAY)
			{
				t3f_set_mouse_xy(t3f_virtual_display_width / 2, t3f_virtual_display_height / 2);
			}
		#endif
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

		if(app->input_type == DOT_INPUT_CONTROLLER)
		{
			app->game.player.ball.x += app->controller.axis_x * 4.0;
			app->game.player.ball.y += app->controller.axis_y * 4.0;
			maybe_activate_shield(app);
		}
		else
		{
			if(app->touch_id > 0)
			{
				update_player_position_touch(app);
				maybe_activate_shield(app);
			}
			else if(app->input_type == DOT_INPUT_MOUSE)
			{
				move_player_with_mouse(app);
				maybe_activate_shield(app);
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

static void open_pause_menu(APP_INSTANCE * app, bool convert)
{
	if(!convert)
	{
		app->game.pause_state = app->game.state;
	}
	t3f_clear_touch_state();
	t3f_clear_mouse_state();
	app->game.state = DOT_GAME_STATE_PAUSE_MENU;
	t3f_reset_gui_input(app->menu[DOT_MENU_PAUSE]);
	if(app->input_type == DOT_INPUT_CONTROLLER)
	{
		t3f_select_next_gui_element(app->menu[DOT_MENU_PAUSE]);
		app->controller.axis_y_pressed = false;
	}
	dot_enable_mouse_cursor(true);
}

static void start_turn(void * data, float x, float y, bool peg_mouse, int touch_id, bool reset_mickeys)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	t3f_play_sample(app->sample[DOT_SAMPLE_GO], 1.0, 0.0, 1.0);
	app->game.state = DOT_GAME_STATE_PLAY;
	app->game.state_tick = 0;
	app->game.player.ball.active = true;
	app->game.player.want_shield = true;
	app->game.player.ball.x = x;
	app->game.player.ball.y = y;
	if(peg_mouse)
	{
		t3f_set_mouse_xy(app->game.player.ball.x, app->game.player.ball.y);
	}
	app->game.level_start = false;
	dot_enable_mouse_cursor(false);
	if(reset_mickeys)
	{
		t3f_get_mouse_mickeys(&i, &i, &i);
	}
	if(touch_id >= 0)
	{
		t3f_use_touch_press(touch_id);
	}
	app->controller.button = false;
}

void dot_gameplay_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int colored = 0;
	int i;

	/* handle shield logic */
	dot_game_shield_logic(data);

	/* move player */
	dot_game_move_player(data);

	/* handle ball logic */
	colored = dot_game_move_balls(data);

	app->game.a_colored_balls_remaining = colored;
	dot_game_check_player_collisions(data);
	app->game.a_oops_ticks++;

	app->game.a_bob_and_weave_ticks++;
	if(!t3f_achievement_gotten(app->achievements, DOT_ACHIEVEMENT_BOB_AND_WEAVE))
	{
		if(app->game.a_bob_and_weave_ticks >= 3600)
		{
			_dot_update_achievement_progress(app, DOT_ACHIEVEMENT_BOB_AND_WEAVE, 1);
		}
	}

	/* move on to next level */
	if(colored == 0)
	{
		if(!t3f_achievement_gotten(app->achievements, DOT_ACHIEVEMENT_GETTING_INTO_IT))
		{
			_dot_update_achievement_progress(app, DOT_ACHIEVEMENT_GETTING_INTO_IT, 1);
		}
		if(!t3f_achievement_gotten(app->achievements, DOT_ACHIEVEMENT_FULL_COMBO))
		{
			if(!app->game.a_combo_broken)
			{
				_dot_update_achievement_progress(app, DOT_ACHIEVEMENT_FULL_COMBO, 1);
			}
		}
		if(!t3f_achievement_gotten(app->achievements, DOT_ACHIEVEMENT_GETTING_GOOD))
		{
			if(app->game.a_start_lives == app->game.lives)
			{
				_dot_update_achievement_progress(app, DOT_ACHIEVEMENT_GETTING_GOOD, 1);
			}
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
		t3f_clear_touch_state();
		dot_enable_mouse_cursor(true);
		app->game.old_bg_color = app->game.bg_color;
		dot_game_setup_level(data, app->game.level + 1);
		app->game.bg_color_fade = 0.0;
		app->game.combo = 0;
		app->game.shield.active = false;
		if(!t3f_achievement_gotten(app->achievements, DOT_ACHIEVEMENT_SEE_IT_THROUGH))
		{
			if(app->game.level >= 10)
			{
				_dot_update_achievement_progress(app, DOT_ACHIEVEMENT_SEE_IT_THROUGH, 1);
			}
		}
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

	/* make lives flash for a certain amount of time */
	if(app->game.extra_life_tick)
	{
		app->game.extra_life_tick--;
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
			if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE) || t3f_key_pressed(ALLEGRO_KEY_BACK))
			{
				open_pause_menu(app, false);
				t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
				t3f_use_key_press(ALLEGRO_KEY_BACK);
			}
			else
			{
				switch(app->input_type)
				{
					case DOT_INPUT_MOUSE:
					{
						if(app->desktop_mode)
						{
							if(t3f_touch_pressed(0) && app->touch_x >= DOT_GAME_TOUCH_START_X && app->touch_x < DOT_GAME_TOUCH_END_X && app->touch_y >= DOT_GAME_TOUCH_START_Y && app->touch_y < DOT_GAME_TOUCH_END_Y)
							{
								start_turn(app, t3f_get_mouse_x(), t3f_get_mouse_y(), false, 0, true);
							}
						}
						else
						{
							if(t3f_touch_pressed(0))
							{
								start_turn(app, DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2, true, 0, true);
							}
						}
						break;
					}
					case DOT_INPUT_TOUCH:
					{
						if(app->touch_id > 0)
						{
							if(t3f_touch_pressed(app->touch_id))
							{
								start_turn(app, DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2, false, app->touch_id, true);
							}
						}
						break;
					}
					case DOT_INPUT_CONTROLLER:
					{
						if(app->controller.button)
						{
							start_turn(app, DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2, false, -1, false);
						}
						break;
					}
				}
				if(app->game.state == DOT_GAME_STATE_PLAY)
				{
					app->game.a_bob_and_weave_ticks = 0;
					app->game.a_oops_ticks = 0;
				}

				/* handle ball logic */
				colored = dot_game_move_balls(data);
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
			if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE) || t3f_key_pressed(ALLEGRO_KEY_BACK))
			{
				app->game.state = app->game.pause_state;
				t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
				t3f_use_key_press(ALLEGRO_KEY_BACK);
			}
			else
			{
				dot_intro_process_menu(app, app->menu[DOT_MENU_PAUSE]);
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
				dot_game_exit(data, false);
			}
			break;
		}

		/* normal game state */
		default:
		{
			dot_enable_mouse_cursor(false);
			if((app->touch_id == 0 && t3f_touch_pressed(app->touch_id)) || t3f_key_pressed(ALLEGRO_KEY_ESCAPE) || t3f_key_pressed(ALLEGRO_KEY_BACK) || app->controller.button || app->controller.current_joy_disconnected)
			{
				if(app->input_type == DOT_INPUT_MOUSE)
				{
					t3f_set_mouse_xy(app->game.player.ball.x, app->game.player.ball.y);
					dot_enable_mouse_cursor(true);
					t3f_use_touch_press(app->touch_id);
				}
				open_pause_menu(app, false);
				t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
				t3f_use_key_press(ALLEGRO_KEY_BACK);
				app->controller.button = false;
			}
			else
			{
				dot_gameplay_logic(data);
			}
			break;
		}
	}
	app->controller.current_joy_disconnected = false;
	app->game.tick++;
}

/* render the HUD */
void dot_game_render_hud(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_COLOR text_color;

	char buffer[256] = {0};
	ALLEGRO_COLOR shadow = al_map_rgba_f(0.0, 0.0, 0.0, 0.25);
	bool held = al_is_bitmap_drawing_held();

	if(held)
	{
		al_hold_bitmap_drawing(false);
	}

	al_draw_filled_rectangle(0, DOT_GAME_PLAYFIELD_HEIGHT, 540 + 0.5, DOT_GAME_PLAYFIELD_HEIGHT + 80 + 0.5, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
	if(app->desktop_mode)
	{
		if(app->state == DOT_STATE_GAME && app->game.state == DOT_GAME_STATE_START)
		{
			al_draw_filled_rectangle(0, DOT_GAME_PLAYFIELD_HEIGHT, 540 + 0.5, DOT_GAME_PLAYFIELD_HEIGHT + 80 + 0.5, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
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
	if(app->game.extra_life_tick && (app->game.extra_life_tick / 6) % 2)
	{
		text_color = al_map_rgba_f(1.0, 1.0, 0.0, 1.0);
	}
	else
	{
		text_color = t3f_color_white;
	}
	dot_shadow_text(app->font[DOT_FONT_32], text_color, shadow, 8, 440 + 40, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, 0, buffer);

	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_EMO_BG], shadow, t3f_virtual_display_width / 2 - 32 + DOT_SHADOW_OX * 2, DOT_GAME_PLAYFIELD_HEIGHT + 40 - 32 + DOT_SHADOW_OY * 2, 0, 64, 64, 0);
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_EMO_BG], app->dot_color[app->game.player.ball.type], t3f_virtual_display_width / 2 - 32, DOT_GAME_PLAYFIELD_HEIGHT + 40 - 32, 0, 64, 64, 0);
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_EMO_NORMAL + app->game.emo_state], t3f_color_white, t3f_virtual_display_width / 2 - 32, DOT_GAME_PLAYFIELD_HEIGHT + 40 - 32, 0.0, 64, 64, 0);
	t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_EMO_FG], t3f_color_white, t3f_virtual_display_width / 2 - 32, DOT_GAME_PLAYFIELD_HEIGHT + 40 - 32, 0, 64, 64, 0);
	al_hold_bitmap_drawing(false);
	al_hold_bitmap_drawing(held);
}

static void dot_create_touch_start_effect(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	float sx = 512.0 / (float)app->menu_view->virtual_width;
	float sy = 512.0 / (float)app->menu_view->virtual_height;
	bool held = al_is_bitmap_drawing_held();

	if(held)
	{
		al_hold_bitmap_drawing(false);
	}
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM | ALLEGRO_STATE_BLENDER);
	al_set_target_bitmap(app->bitmap[DOT_BITMAP_SCRATCH]->bitmap);
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
	al_set_clipping_rectangle(0, 0, 512, 512);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 1.0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
	al_draw_filled_rectangle(DOT_GAME_TOUCH_START_X * sx, DOT_GAME_TOUCH_START_Y * sy, DOT_GAME_TOUCH_END_X * sx, DOT_GAME_TOUCH_END_Y * sy, al_map_rgba_f(1.0, 1.0, 1.0, 1.0));
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
	float level_y = 8;
	float start_y = DOT_GAME_PLAYFIELD_HEIGHT / 2;
	char * touch_text[2];

	if(app->input_type == DOT_INPUT_MOUSE)
	{
		if(app->desktop_mode)
		{
			touch_text[0] = "Click";
			touch_text[1] = "Here";
		}
		else
		{
			touch_text[0] = "Click";
			touch_text[1] = "Anywhere";
		}
	}
	else if(app->input_type == DOT_INPUT_CONTROLLER)
	{
		touch_text[0] = "Press";
		touch_text[1] = "Button";
	}
	else
	{
		touch_text[0] = "Touch";
		touch_text[1] = "Screen";
	}
	if(!app->desktop_mode)
	{
		level_y = DOT_GAME_PLAYFIELD_HEIGHT / 2.0 - t3f_get_font_line_height(app->font[DOT_FONT_32]) / 2;
	}
	start_y = app->menu_view->virtual_height / 2.0;
	t3f_select_view(t3f_default_view);
	al_clear_to_color(app->game.bg_color);
	t3f_select_view(app->main_view);
	al_hold_bitmap_drawing(true);
	dot_bg_objects_render(data);
	al_draw_bitmap(app->bitmap[DOT_BITMAP_BG]->bitmap, 0, 0, 0);
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
	if(app->game.player.ball.active)
	{
		cx = (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]->bitmap) / 2);
		cy = (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_RED + app->game.player.ball.type]->bitmap) / 2);
		ecx = (float)(al_get_bitmap_width(app->bitmap[DOT_BITMAP_BALL_EYES]->bitmap) / 2);
		ecy = (float)(al_get_bitmap_height(app->bitmap[DOT_BITMAP_BALL_EYES]->bitmap) / 2);
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
	qsort(app->splat_particle, app->splat_particles, sizeof(DOT_PARTICLE *), dot_particle_qsort_helper);
	for(i = 0; i < app->splat_particles; i++)
	{
		dot_particle_render(app->splat_particle[i], app->bitmap[DOT_BITMAP_PARTICLE]);
	}
	dot_game_render_hud(data);
	if((app->game.tick / 6) % 2)
	{
		text_color = al_map_rgba_f(1.0, 1.0, 0.0, 1.0);
	}
	al_hold_bitmap_drawing(false);
	if(app->game.state == DOT_GAME_STATE_PAUSE_MENU)
	{
		t3f_select_view(app->main_view);
		al_draw_filled_rectangle(0.0, 0.0, app->main_view->virtual_width + 0.5, app->main_view->virtual_height + 0.5, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		al_hold_bitmap_drawing(true);
		t3f_select_view(app->main_view);
		if(!app->desktop_mode)
		{
			dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, level_y, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, "Paused");
		}
		t3f_select_view(app->menu_view);
		t3f_render_gui(app->menu[DOT_MENU_PAUSE], 0);
	}
	else if(app->game.state == DOT_GAME_STATE_START)
	{
		if(!app->desktop_mode)
		{
			al_draw_filled_rectangle(0.0, 0.0, t3f_virtual_display_width, DOT_GAME_PLAYFIELD_HEIGHT + 80, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
		}
		t3f_select_view(app->menu_view);
		al_hold_bitmap_drawing(true);
		dot_create_touch_start_effect(data);
		if(app->desktop_mode)
		{
			t3f_set_clipping_rectangle(0, 0, DOT_GAME_PLAYFIELD_WIDTH, DOT_GAME_PLAYFIELD_HEIGHT);
			t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_SCRATCH], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 0, 0, 0.0, DOT_GAME_PLAYFIELD_WIDTH + 0.5, app->menu_view->virtual_height, 0);
		}
		if(app->desktop_mode)
		{
			al_hold_bitmap_drawing(false);
			al_hold_bitmap_drawing(true);
			t3f_set_clipping_rectangle(0, 0, 0, 0);
		}
		if(app->game.level_start)
		{
			sprintf(buf, "Level %d", app->game.level + 1);
			t3f_select_view(app->main_view);
			dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, level_y, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, buf);
		}
		t3f_select_view(app->menu_view);
		dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, start_y - t3f_get_font_line_height(app->font[DOT_FONT_32]), DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, touch_text[0]);
		dot_shadow_text(app->font[DOT_FONT_32], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, start_y, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, touch_text[1]);
	}
	al_hold_bitmap_drawing(false);
}
