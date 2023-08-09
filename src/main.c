#include "t3f/t3f.h"
#include "t3f/resource.h"
#include "t3f/music.h"
#include "t3f/view.h"
#include "t3f/draw.h"
#include "t3f/input.h"
#include "t3net/leaderboard.h"
#include "avc/avc.h"
#include "instance.h"
#include "intro.h"
#include "game.h"
#include "leaderboard.h"
#include "privacy.h"
#include "input.h"

static bool dot_show_touch_hand = false;
static int dot_screenshot_count = 0;

static ALLEGRO_BITMAP * dot_create_scratch_bitmap(int w, int h)
{
	ALLEGRO_STATE old_state;
	ALLEGRO_BITMAP * bp;

	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_NO_PRESERVE_TEXTURE);
	bp = al_create_bitmap(w, h);
	al_restore_state(&old_state);

	return bp;
}

static void disable_controller(APP_INSTANCE * app)
{
	if(app->using_controller)
	{
		if(app->state == DOT_STATE_GAME)
		{
			t3f_set_mouse_xy(app->game.player.ball.x, app->game.player.ball.y);
		}
		app->using_controller = false;
	}
}

static void dot_event_handler(ALLEGRO_EVENT * event, void * data)
{
	ALLEGRO_STATE old_state;
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
		{
			al_destroy_bitmap(app->bitmap[DOT_BITMAP_SCRATCH]);
			t3f_event_handler(event);
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
		{
			t3f_event_handler(event);
			al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
			al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_NO_PRESERVE_TEXTURE);
			app->bitmap[DOT_BITMAP_SCRATCH] = dot_create_scratch_bitmap(DOT_BITMAP_SCRATCH_WIDTH, DOT_BITMAP_SCRATCH_HEIGHT);
			al_restore_state(&old_state);
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			t3f_event_handler(event);
			app->want_disable_controller = true;
			break;
		}

		/* pass the event through to T3F for handling by default */
		default:
		{
			t3f_event_handler(event);
			dot_handle_joystick_event(&app->controller, event);
			break;
		}
	}
}

void app_touch_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	bool need_click = false;
	int i;

	if(!app->desktop_mode)
	{
		app->touch_id = -1;
		if(app->touch_id < 0)
		{
			for(i = 0; i < T3F_MAX_TOUCHES; i++)
			{
				if(t3f_touch[i].active)
				{
					app->touch_id = i;
					break;
				}
			}
		}
		if(app->touch_id >= 0)
		{
			app->touch_x = t3f_touch[app->touch_id].x;
			app->touch_y = t3f_touch[app->touch_id].y - 520;
			disable_controller(app);
		}
	}
	else
	{
		app->touch_id = -1;
		if(t3f_mouse_button[1] && app->game.state == DOT_GAME_STATE_PLAY)
		{
			app->game.state = DOT_GAME_STATE_PAUSE;
			al_show_mouse_cursor(t3f_display);
		}
		if(app->state == DOT_STATE_GAME && (app->game.state == DOT_GAME_STATE_PAUSE || app->game.state == DOT_GAME_STATE_START))
		{
			need_click = true;
		}
		if(!t3f_mouse_hidden && (!need_click || t3f_mouse_button[0]))
		{
			app->touch_id = 0;
		}
		app->touch_x = t3f_mouse_x;
		app->touch_y = t3f_mouse_y;
	}
}

static int particle_qsort_helper(const void * p1, const void * p2)
{
	DOT_PARTICLE ** sp1;
	DOT_PARTICLE ** sp2;

	sp1 = (DOT_PARTICLE **)p1;
	sp2 = (DOT_PARTICLE **)p2;

	return (*sp1)->z < (*sp2)->z;
}

void app_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;
	short touch_x, touch_y;
	bool frame;

	if(app->demo_file)
	{
		/* convert touch data to integer for demo operations */
		touch_x = t3f_mouse_x;
		touch_y = t3f_mouse_y;
		t3f_touch[0].x = touch_x;
		t3f_touch[0].y = touch_y;

		/* load touch data from file if we aren't recording */
		if(!app->demo_recording)
		{
			frame = al_fgetc(app->demo_file);
			if(frame)
			{
				t3f_touch[0].active = al_fgetc(app->demo_file);
				t3f_touch[0].x = al_fread16le(app->demo_file);
				t3f_touch[0].y = al_fread16le(app->demo_file);
				t3f_mouse_x = t3f_touch[0].x;
				t3f_mouse_y = t3f_touch[0].y;
				t3f_mouse_button[0] = t3f_touch[0].active;
			}
			else
			{
				app->demo_done = true;
				t3f_exit();
			}
		}

		/* write touch data to file if we are recording */
		else
		{
			al_fputc(app->demo_file, 1);
			al_fputc(app->demo_file, t3f_touch[0].active);
			al_fwrite16le(app->demo_file, touch_x);
			al_fwrite16le(app->demo_file, touch_y);
		}
	}

	/* capture screenshot */
	if(t3f_key[ALLEGRO_KEY_PRINTSCREEN] || t3f_key[ALLEGRO_KEY_TILDE] || t3f_key[104])
	{
		char buf[32];

		sprintf(buf, "screen_%d.png", dot_screenshot_count);
		al_save_bitmap(buf, al_get_backbuffer(t3f_display));
		dot_screenshot_count++;
		t3f_key[ALLEGRO_KEY_PRINTSCREEN] = 0;
		t3f_key[ALLEGRO_KEY_TILDE] = 0;
		t3f_key[104] = 0;
	}

	if(app->want_disable_controller)
	{
		disable_controller(app);
		app->want_disable_controller = false;
		t3f_select_input_view(t3f_current_view);
	}
	app_touch_logic(data);
	if(!app->entering_name)
	{
		dot_read_input(&app->controller);
	}
	if(app->controller.axis_x != 0.0 || app->controller.axis_y != 0.0 || app->controller.button)
	{
		app->using_controller = true;
	}
/*	if(app->button)
	{
		if(app->controller.axis_x != 0.0 || app->controller.axis_y != 0.0)
		{
			app->controller.dead_zone = fabs(app->controller.axis_x);
			if(fabs(app->controller.axis_y) > app->controller.dead_zone)
			{
				app->controller.dead_zone = fabs(app->controller.axis_y);
			}
			app->controller.dead_zone += 0.05;
			printf("dead zone: %f\n", app->controller.dead_zone);
		}
	} */
	for(i = 0; i < T3F_MAX_TOUCHES; i++)
	{
		if(t3f_touch[i].active)
		{
			disable_controller(app);
		}
	}
	switch(app->state)
	{
		case DOT_STATE_INTRO:
		{
			dot_intro_logic(data);
			break;
		}
		case DOT_STATE_LEADERBOARD:
		{
			dot_leaderboard_logic(data);
			break;
		}
		case DOT_STATE_PRIVACY:
		{
			dot_privacy_logic(data);
			break;
		}
		case DOT_STATE_GAME:
		{
			dot_game_logic(data);
			break;
		}
	}
	app->active_particles = 0;
	for(i = 0; i < DOT_MAX_PARTICLES; i++)
	{
		if(app->particle[i].active)
		{
			app->active_particle[app->active_particles] = &app->particle[i];
			app->active_particles++;
		}
		dot_particle_logic(&app->particle[i]);
	}
}

static void show_joystick_data(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;
	int pos_y = 0;

	for(i = 0; i < app->controller.input_handler->elements; i++)
	{
		t3f_draw_textf(app->font[DOT_FONT_8], t3f_color_white, 0, pos_y, 0, 0, "%1.1f %d", app->controller.input_handler->element[i].val, app->controller.input_handler->element[i].held);
		pos_y += t3f_get_font_line_height(app->font[DOT_FONT_8]);
	}
}

void app_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	switch(app->state)
	{
		case DOT_STATE_INTRO:
		{
			dot_intro_render(data);
			break;
		}
		case DOT_STATE_LEADERBOARD:
		{
			dot_leaderboard_render(data);
			break;
		}
		case DOT_STATE_PRIVACY:
		{
			dot_privacy_render(data);
			break;
		}
		case DOT_STATE_GAME:
		{
			dot_game_render(data);
			break;
		}
	}
	al_hold_bitmap_drawing(true);
	qsort(app->active_particle, app->active_particles, sizeof(DOT_PARTICLE *), particle_qsort_helper);
	for(i = 0; i < app->active_particles; i++)
	{
		dot_particle_render(app->active_particle[i], app->bitmap[DOT_BITMAP_PARTICLE]);
	}
	if(app->desktop_mode)
	{
		float ox = 0, oy = 0;

		if(dot_show_touch_hand && t3f_mouse_y >= t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT)
		{
			ox = 12.0;
			oy = -12.0;
			t3f_draw_rotated_bitmap(app->bitmap[DOT_BITMAP_HAND], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 92, 24, t3f_mouse_x, t3f_mouse_y, 0, 0, 0);
			al_draw_rotated_bitmap(t3f_mouse_button[0] ? app->bitmap[DOT_BITMAP_HAND_DOWN] : app->bitmap[DOT_BITMAP_HAND], 92, 24, t3f_mouse_x + ox, t3f_mouse_y + oy, 0, 0);
		}
	}
	al_hold_bitmap_drawing(false);
//	show_joystick_data(app);
}

static bool dot_load_bitmap(APP_INSTANCE * app, int bitmap, const char * fn, int size)
{
	char buf[256];
	int i;

	for(i = size; i >= 1; i--)
	{
		sprintf(buf, "data/graphics/%dx/%s", i, fn);
		t3f_load_resource((void **)&app->bitmap[bitmap], t3f_bitmap_resource_handler_proc, buf, 0, 0, 0);
		if(app->bitmap[bitmap])
		{
			return true;
		}
	}
	printf("Failed to load image %d (%s)!\n", bitmap, fn);
	return false;
}

static bool dot_load_font(APP_INSTANCE * app, int font, const char * fn, int size)
{
	t3f_load_resource((void **)&app->font[font], t3f_font_resource_handler_proc, fn, size, 0, 0);
	if(!app->font[font])
	{
		printf("Failed to load font %d!\n", font);
		return false;
	}
	return true;
}

static ALLEGRO_COLOR dot_get_ball_color(ALLEGRO_BITMAP * bp)
{
	ALLEGRO_COLOR c;

	al_lock_bitmap(bp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
	c = al_get_pixel(bp, al_get_bitmap_width(bp) / 2, al_get_bitmap_height(bp) / 2);
	al_unlock_bitmap(bp);
	return c;
}

bool app_avc_init_proc(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->demo_file = al_fopen(app->demo_filename, "rb");
	if(!app->demo_file)
	{
		printf("Failed to open demo file.\n");
		return false;
	}
	app->demo_seed = al_fread32le(app->demo_file);
	t3f_srand(&app->rng_state, app->demo_seed);
	app->demo_done = false;
	app->state = 0;
	return true;
}

bool app_avc_logic_proc(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app_logic(data);
	if(app->demo_done)
	{
		al_fclose(app->demo_file);
		app->demo_file = NULL;
		return false;
	}
	return true;
}

static void free_graphics(APP_INSTANCE * app)
{
	int i;

	for(i = 0; i < DOT_MAX_BITMAPS; i++)
	{
		if(app->bitmap[i])
		{
			if(!t3f_destroy_resource(app->bitmap[i]))
			{
				al_destroy_bitmap(app->bitmap[i]);
			}
			app->bitmap[i] = NULL;
		}
	}
	for(i = 0; i < DOT_MAX_FONTS; i++)
	{
		if(app->font[i])
		{
			if(!t3f_destroy_resource(app->font[i]))
			{
				t3f_destroy_font(app->font[i]);
			}
			app->font[i] = NULL;
		}
	}
	if(app->atlas)
	{
		t3f_destroy_atlas(app->atlas);
		app->atlas = NULL;
	}
}

static bool load_graphics(APP_INSTANCE * app)
{
	char fn_buf[1024];
	int i;

	/* load graphics set */
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_RED, "ball_red.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_GREEN, "ball_green.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_BLUE, "ball_blue.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_PURPLE, "ball_purple.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_YELLOW, "ball_yellow.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_ORANGE, "ball_orange.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_BLACK, "ball_black.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_EYES, "ball_eyes.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_COMBO, "combo.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_PARTICLE, "particle.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_BG, "emo_bg.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_FG, "emo_fg.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_NORMAL, "emo_normal.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_BLINK, "emo_blink.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_WOAH, "emo_woah.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_DEAD, "emo_dead.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_TARGET, "target.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BG, "bg.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_LOGO, "logo.png", app->graphics_size_multiplier))
	{
		goto fail;
	}
	app->bitmap[DOT_BITMAP_SCRATCH] = dot_create_scratch_bitmap(DOT_BITMAP_SCRATCH_WIDTH, DOT_BITMAP_SCRATCH_HEIGHT);
	if(!app->bitmap[DOT_BITMAP_SCRATCH])
	{
		printf("Failed to create effects scratch bitmap!\n");
		goto fail;
	}

	/* build atlas from graphics set */
	app->atlas = t3f_create_atlas(512 * app->graphics_size_multiplier, 512 * app->graphics_size_multiplier);
	if(!app->atlas)
	{
		goto fail;
	}
	for(i = 0; i < DOT_BITMAP_ATLAS_BOUNDARY; i++)
	{
		if(app->bitmap[i])
		{
			t3f_add_bitmap_to_atlas(app->atlas, &app->bitmap[i], T3F_ATLAS_SPRITE);
		}
	}

	/* load fonts */
	sprintf(fn_buf, "data/fonts/kongtext_%dx.ini", app->graphics_size_multiplier);
	if(!dot_load_font(app, DOT_FONT_14, fn_buf, 14))
	{
		printf("Failed to load font %d!\n", DOT_FONT_8);
		goto fail;
	}
	if(!dot_load_font(app, DOT_FONT_8, fn_buf, 8))
	{
		printf("Failed to load font %d!\n", DOT_FONT_8);
		goto fail;
	}
	if(!dot_load_font(app, DOT_FONT_16, fn_buf, 16))
	{
		printf("Failed to load font %d!\n", DOT_FONT_16);
		goto fail;
	}
	if(!dot_load_font(app, DOT_FONT_32, fn_buf, 32))
	{
		printf("Failed to load font %d!\n", DOT_FONT_32);
		goto fail;
	}

	return true;

	fail:
	{
		free_graphics(app);
		return false;
	}
}

bool app_load_data(APP_INSTANCE * app)
{
	const char * val;

	/* use manually configured graphics size multiplier if setting found */
	val = al_get_config_value(t3f_config, "Game Data", "Graphics Size Multiplier");
	if(val)
	{
		app->graphics_size_multiplier = atoi(val);
	}

	/* otherwise, try to detect the optimal size */
	else
	{
		app->graphics_size_multiplier = al_get_display_width(t3f_display) / t3f_virtual_display_width;
		if(app->graphics_size_multiplier > al_get_display_height(t3f_display) / t3f_virtual_display_height)
		{
			app->graphics_size_multiplier = al_get_display_height(t3f_display) / t3f_virtual_display_height;
		}
	}

	/* keep graphics size multiplier within acceptable range */
	if(app->graphics_size_multiplier < 1)
	{
		app->graphics_size_multiplier = 1;
	}
	else if(app->graphics_size_multiplier > 5)
	{
		app->graphics_size_multiplier = 5;
	}

	/* load graphics */
	for(; app->graphics_size_multiplier >= 1; app->graphics_size_multiplier--)
	{
		if(load_graphics(app))
		{
			break;
		}
	}
	if(app->graphics_size_multiplier < 1)
	{
		return false;
	}

	/* load sounds */
	app->sample[DOT_SAMPLE_START] = al_load_sample("data/sounds/sfx0005.wav");
	if(!app->sample[DOT_SAMPLE_START])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_START);
		return false;
	}
	app->sample[DOT_SAMPLE_GRAB] = al_load_sample("data/sounds/sfx0000.wav");
	if(!app->sample[DOT_SAMPLE_GRAB])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_GRAB);
		return false;
	}
	app->sample[DOT_SAMPLE_LOSE] = al_load_sample("data/sounds/sfx0001.wav");
	if(!app->sample[DOT_SAMPLE_LOSE])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_LOSE);
		return false;
	}
	app->sample[DOT_SAMPLE_GO] = al_load_sample("data/sounds/sfx0004.wav");
	if(!app->sample[DOT_SAMPLE_GO])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_GO);
		return false;
	}
	app->sample[DOT_SAMPLE_SCORE] = al_load_sample("data/sounds/score_up.wav");
	if(!app->sample[DOT_SAMPLE_SCORE])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_SCORE);
		return false;
	}
	return true;
}

/* read user preferences and other data */
void app_read_user_data(APP_INSTANCE * app)
{
	const char * val;

	/* see if we need to run setup */
	app->first_run = true;
	val = al_get_config_value(t3f_user_data, "Game Data", "Setup Done");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			app->first_run = false;
		}
	}

	/* load high score */
	val = al_get_config_value(t3f_user_data, "Game Data", "High Score");
	if(val)
	{
		app->game.high_score = atoi(val);
	}
	else
	{
		app->game.high_score = 0;
	}

	/* load user key */
	val = al_get_config_value(t3f_user_data, "Game Data", "User Key");
	if(val)
	{
		strcpy(app->user_key, val);
	}

	/* load user name */
	val = al_get_config_value(t3f_user_data, "Game Data", "User Name");
	if(val)
	{
		strcpy(app->user_name, val);
	}
	else
	{
		strcpy(app->user_name, "Anonymous");
	}

	/* load Steam user display name */
	val = t3f_get_steam_user_display_name();
	if(val)
	{
		al_set_config_value(t3f_user_data, "Game Data", "Steam User Display Name", val);
	}
	else
	{
		val = al_get_config_value(t3f_user_data, "Game Data", "Steam User Display Name");
	}
	if(val)
	{
		strcpy(app->user_name, val);
	}

	app->upload_scores = false;
	val = al_get_config_value(t3f_user_data, "Game Data", "Upload Scores");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			app->upload_scores = true;
		}
	}

	app->music_enabled = false;
	val = al_get_config_value(t3f_user_data, "Game Data", "Music Enabled");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			app->music_enabled = true;
		}
	}

	/* user cheat settings */
	app->game.cheats_enabled = false;
	app->game.speed_multiplier = 1.0;
	val = al_get_config_value(t3f_user_data, "Game Data", "speed_multiplier");
	if(val)
	{
		app->game.speed_multiplier = atof(val);
		app->game.cheats_enabled = true;
	}
	app->game.start_level = 0;
	val = al_get_config_value(t3f_user_data, "Game Data", "start_level");
	if(val)
	{
		app->game.start_level = atoi(val);
		if(app->game.start_level != 0)
		{
			app->game.cheats_enabled = true;
		}
	}
	app->game.start_lives = 3;
	val = al_get_config_value(t3f_user_data, "Game Data", "start_lives");
	if(val)
	{
		app->game.start_lives = atoi(val);
		if(app->game.start_lives != 3)
		{
			app->game.cheats_enabled = true;
		}
	}

	/* set up leaderboard URLs */
	val = al_get_config_value(t3f_user_data, "Game Data", "leaderboard_get_user_key_url");
	if(!val)
	{
		val = "https://www.tcubedsoftware.com/scripts/leaderboards/get_user_key.php";
		al_set_config_value(t3f_user_data, "Game Data", "leaderboard_get_user_key_url", val);
	}
	strcpy(app->leaderboard_get_user_key_url, val);
	val = al_get_config_value(t3f_user_data, "Game Data", "leaderboard_set_user_name_url");
	if(!val)
	{
		val = "https://www.tcubedsoftware.com/scripts/leaderboards/set_user_name.php";
		al_set_config_value(t3f_user_data, "Game Data", "leaderboard_set_user_name_url", val);
	}
	strcpy(app->leaderboard_set_user_name_url, val);
	val = al_get_config_value(t3f_user_data, "Game Data", "leaderboard_submit_url");
	if(!val)
	{
		val = "https://www.tcubedsoftware.com/scripts/leaderboards/update.php";
		al_set_config_value(t3f_user_data, "Game Data", "leaderboard_submit_url", val);
	}
	strcpy(app->leaderboard_submit_url, val);
	val = al_get_config_value(t3f_user_data, "Game Data", "leaderboard_retrieve_url");
	if(!val)
	{
		val = "https://www.tcubedsoftware.com/scripts/leaderboards/query.php";
		al_set_config_value(t3f_user_data, "Game Data", "leaderboard_retrieve_url", val);
	}
	strcpy(app->leaderboard_retrieve_url, val);
}

/* read config for local machine */
void app_read_config(APP_INSTANCE * app)
{
	const char * val;

	app->controller.dead_zone = 0.15;
	val = al_get_config_value(t3f_config, "Game Data", "Controller Dead Zone");
	if(val)
	{
		app->controller.dead_zone = atof(val);
	}
}

bool app_process_arguments(APP_INSTANCE * app, int argc, char * argv[])
{
	int i;

	/* see if we want to record a demo */
	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "--record_demo"))
		{
			if(argc <= i + 1)
			{
				printf("Missing argument.\n");
				return false;
			}
			else
			{
				if(!dot_load_bitmap(app, DOT_BITMAP_HAND, "hand.png", 1))
				{
					return false;
				}
				if(!dot_load_bitmap(app, DOT_BITMAP_HAND_DOWN, "hand_down.png", 1))
				{
					return false;
				}
				app->demo_file = al_fopen(argv[i + 1], "wb");
				if(!app->demo_file)
				{
					printf("Failed to open demo file.\n");
					return false;
				}
				dot_show_touch_hand = true;
				app->demo_recording = true;
				app->demo_seed = time(0);
				al_fwrite32le(app->demo_file, app->demo_seed);
			}
		}
		if(!strcmp(argv[i], "--play_demo"))
		{
			if(argc <= i + 1)
			{
				printf("Missing argument.\n");
				return false;
			}
			else
			{
				app->demo_filename = argv[i + 1];
				dot_show_touch_hand = true;
			}
			app_avc_init_proc(app);
		}
		if(!strcmp(argv[i], "--capture_demo"))
		{
			if(argc <= i + 1)
			{
				printf("Missing argument.\n");
				return false;
			}
			else
			{
				if(!dot_load_bitmap(app, DOT_BITMAP_HAND, "hand.png", 1))
				{
					return false;
				}
				if(!dot_load_bitmap(app, DOT_BITMAP_HAND_DOWN, "hand_down.png", 1))
				{
					return false;
				}
				app->demo_filename = argv[i + 1];
				dot_show_touch_hand = true;
			}
			if(!avc_start_capture(t3f_display, "myvideo.mp4", app_avc_init_proc, app_avc_logic_proc, app_render, 60, 0, app))
			{
				printf("Capture failed!\n");
				return false;
			}
		}
	}
	return true;
}

void app_check_mobile_argument(APP_INSTANCE * app, int argc, char * argv[])
{
	int i;

	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "--mobile"))
		{
			app->desktop_mode = false;
			break;
		}
	}
}

static void set_optimal_display_size(APP_INSTANCE * app)
{
	ALLEGRO_MONITOR_INFO info;
	int width, height;
	int current_width = DOT_DISPLAY_WIDTH;
	int current_height = DOT_DISPLAY_HEIGHT;

	al_get_monitor_info(0, &info);
	width = info.x2 - info.x1 - 64;
	height = info.y2 - info.y1 - 64;
	while(current_width <= width && current_height <= height)
	{
		current_width += DOT_DISPLAY_WIDTH / 4;
		current_height += DOT_DISPLAY_HEIGHT / 4;
	}
	current_width -= DOT_DISPLAY_WIDTH / 4;
	current_height -= DOT_DISPLAY_HEIGHT / 4;
	if(current_width != al_get_display_width(t3f_display) || current_height != al_get_display_height(t3f_display))
	{
		t3f_set_gfx_mode(current_width, current_height, t3f_flags);
		al_set_window_position(t3f_display, (info.x2 - info.x1) / 2 - current_width / 2, (info.y2 - info.y1) / 2 - current_height / 2);
	}
}

static void add_particle_list_item(DOT_PARTICLE_LIST * lp, float x, float y)
{
	if(lp->items < DOT_MAX_PARTICLE_LIST_ITEMS)
	{
		lp->item[lp->items].x = x;
		lp->item[lp->items].y = y;
		lp->items++;
	}
}

static bool create_particle_lists(APP_INSTANCE * app)
{
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	ALLEGRO_COLOR c;
	int i, j, k, w, h;
	unsigned char r, g, b, a;
	char buf[16] = {0};
	T3F_FONT * font = NULL;;
	ALLEGRO_BITMAP * scratch = NULL;

	font = t3f_load_resource((void **)&font, t3f_font_resource_handler_proc, "data/fonts/kongtext_1x.ini", 16, 0, 0);
	if(!font)
	{
		goto fail;
	}
	scratch = al_create_bitmap(DOT_BITMAP_SCRATCH_WIDTH, DOT_BITMAP_SCRATCH_HEIGHT);
	if(!scratch)
	{
		goto fail;
	}
	for(i = 0; i < 10; i++)
	{
		app->number_particle_list[i].items = 0;
		sprintf(buf, "%d", i);
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM);
		al_set_target_bitmap(scratch);
		al_identity_transform(&identity);
		al_use_transform(&identity);
		al_set_clipping_rectangle(0, 0, 512, 512);
		al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
		t3f_draw_text(font, t3f_color_white, 0, 0, 0, 0, buf);
		t3f_set_clipping_rectangle(0, 0, 0, 0);
		al_restore_state(&old_state);
		al_lock_bitmap(scratch, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
		w = t3f_get_text_width(font, buf);
		h = t3f_get_font_line_height(font);
		for(j = 0; j < w; j++)
		{
			for(k = 0; k < h; k++)
			{
				c = al_get_pixel(scratch, j, k);
				al_unmap_rgba(c, &r, &g, &b, &a);
				if(a > 192)
				{
					add_particle_list_item(&app->number_particle_list[i], j, k);
				}
			}
		}
		al_unlock_bitmap(scratch);
	}
	if(!t3f_destroy_resource(font))
	{
		t3f_destroy_font(font);
	}
	return true;

	fail:
	{
		if(scratch)
		{
			al_destroy_bitmap(scratch);
		}
		if(font)
		{
			if(!t3f_destroy_resource(font))
			{
				t3f_destroy_font(font);
			}
		}
		return false;
	}
}

bool dot_initialize_achievements(APP_INSTANCE * app)
{
	app->achievements = t3f_create_achievements_list(8);
	if(!app->achievements)
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_LEVEL_COMPLETED, "LEVEL_COMPLETED", "Full Combo", "Complete a level without breaking your combo.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_FULL_COMBO, "FULL_COMBO", "Getting Into It", "Complete a level by clearing the board of all colored dots.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_BOB_AND_WEAVE, "BOB_AND_WEAVE", "Bob and Weave", "Survive for one minute without touching any dots.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_GETTING_GOOD, "GETTING_GOOD", "Getting Good", "Complete a level without losing a life.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_OOPS, "OOPS", "Oops!", "Die within one second of starting a life.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_SEE_IT_THROUGH, "SEE_IT_THROUGH", "See It Through", "Complete the first loop by beating level 10.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_SO_CLOSE, "SO_CLOSE", "So Close!", "Die with only one colored dot remaining.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_GOOD_GAME, "GOOD_GAME", "Good Game", "Score 100,000 points.", 1, false))
	{
		goto fail;
	}
	t3f_load_achievements_data(app->achievements, t3f_user_data, "Achievements");
	return true;

	fail:
	{
		if(app->achievements)
		{
			t3f_destroy_achievements_list(app->achievements);
			app->achievements = NULL;
		}
		return false;
	}
}

void dot_deinitialize_achievements(APP_INSTANCE * app)
{
	if(app->achievements)
	{
		t3f_save_achievements_data(app->achievements, t3f_user_data, "Achievements");
		t3f_destroy_achievements_list(app->achievements);
	}
}

/* initialize our app, load graphics, etc. */
bool app_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	int i;
	char * val;

	/* detect game type */
	app->desktop_mode = false;
	#ifndef ALLEGRO_ANDROID
		app->desktop_mode = true;
	#endif
	app_check_mobile_argument(app, argc, argv);

	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, DOT_DISPLAY_WIDTH, DOT_DISPLAY_HEIGHT, 60.0, app_logic, app_render, T3F_DEFAULT | T3F_USE_FIXED_PIPELINE | T3F_USE_FULLSCREEN, app))
	{
		printf("Error initializing T3F\n");
		return false;
	}
	if(!t3f_init_steam_integration())
	{
		printf("Steam not running!\n");
	}
	if(!dot_initialize_input(&app->controller))
	{
		printf("Error initializing input system!\n");
		return false;
	}
	if(!dot_initialize_achievements(app))
	{
		printf("Error initializing achievements list!\n");
		return false;
	}
	t3f_set_event_handler(dot_event_handler);
	if(!(t3f_flags & T3F_USE_FULLSCREEN))
	{
		set_optimal_display_size(app);
	}
	#ifdef ALLEGRO_ANDROID
		t3net_setup(t3f_run_url, al_path_cstr(t3f_temp_path, '/'));
	#else
		t3net_setup(NULL, al_path_cstr(t3f_temp_path, '/'));
	#endif
	if(!create_particle_lists(app))
	{
		printf("Failed to generate data for particle effects!\n");
		return false;
	}
	if(!app_load_data(app))
	{
		printf("Failed to load data!\n");
		return false;
	}

	app_read_config(app);
	app_read_user_data(app);
	if(app->upload_scores && strlen(app->user_key) < 1)
	{
		val = t3net_get_new_leaderboard_user_key(app->leaderboard_get_user_key_url, NULL);
		if(val)
		{
			al_set_config_value(t3f_user_data, "Game Data", "User Key", val);
			free(val);
			t3f_save_user_data();
		}
	}

	if(!dot_intro_initialize(app))
	{
		printf("Failed to create menu!\n");
		return false;
	}

	/* change view focus so 3D effects look right */
	t3f_adjust_view(t3f_current_view, t3f_current_view->offset_x, t3f_current_view->offset_y, t3f_current_view->width, t3f_current_view->height, DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2, t3f_current_view->flags);

	/* create color tables */
	for(i = 0; i <= DOT_BITMAP_BALL_BLACK; i++)
	{
		app->dot_color[i] = dot_get_ball_color(app->bitmap[i]);
	}
	app->level_color[0] = al_map_rgb(47, 151, 218);
	app->level_color[1] = al_map_rgb(48, 196, 218);
	app->level_color[2] = al_map_rgb(48, 218, 192);
	app->level_color[3] = al_map_rgb(48, 218, 134);
	app->level_color[4] = al_map_rgb(147, 186, 33);
	app->level_color[5] = al_map_rgb(186, 178, 33);
	app->level_color[6] = al_map_rgb(218, 175, 48);
	app->level_color[7] = al_map_rgb(218, 123, 48);
	app->level_color[8] = al_map_rgb(218, 48, 48);
	app->level_color[9] = al_map_rgb(204, 48, 218);
	app->controller.current_joy = -1;
	app->controller.current_joy_handle = NULL;

	dot_setup_bg_objects(app);
	dot_setup_credits(&app->credits);

	dot_intro_setup(app);
	if(app->music_enabled)
	{
		t3f_play_music(DOT_MUSIC_TITLE);
	}
	if(!app->desktop_mode)
	{
		app->menu_showing = true;
	}
	if(app->first_run)
	{
		app->current_menu = DOT_MENU_UPLOAD_SCORES;
	}
	app->state = DOT_STATE_INTRO;

	if(!app_process_arguments(app, argc, argv))
	{
		printf("Failed to process command line arguments!\n");
		return false;
	}
	return true;
}

void app_exit(APP_INSTANCE * app)
{
	int i;

	if(app->demo_file)
	{
		al_fputc(app->demo_file, 0);
		al_fclose(app->demo_file);
	}
	free_graphics(app);
	for(i = 0; i < DOT_MAX_SAMPLES; i++)
	{
		if(app->sample[i])
		{
			al_destroy_sample(app->sample[i]);
		}
	}
	for(i = 0; i < DOT_MAX_MENUS; i++)
	{
		if(app->menu[i])
		{
			t3f_destroy_gui(app->menu[i]);
		}
	}
	if(app->leaderboard)
	{
		t3net_destroy_leaderboard(app->leaderboard);
	}
	dot_deinitialize_achievements(app);
	dot_destroy_input(&app->controller);
	t3f_shutdown_steam_integration();
}

int main(int argc, char * argv[])
{
	APP_INSTANCE app;

	memset(&app, 0, sizeof(APP_INSTANCE));
	if(app_initialize(&app, argc, argv))
	{
		t3f_run();
	}
	else
	{
		printf("Error: could not initialize game!\n");
	}
	app_exit(&app);
	t3f_finish();
	return 0;
}
