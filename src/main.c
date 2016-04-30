#include "t3f/t3f.h"
#include "t3f/resource.h"
#include "t3f/music.h"
#include "t3f/view.h"
#include "instance.h"
#include "intro.h"
#include "game.h"
#include "leaderboard.h"

void app_touch_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

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
	}
}

void app_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	app_touch_logic(data);
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
		case DOT_STATE_GAME:
		{
			dot_game_logic(data);
			break;
		}
	}
	for(i = 0; i < DOT_MAX_PARTICLES; i++)
	{
		dot_particle_logic(&app->particle[i]);
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
		case DOT_STATE_GAME:
		{
			dot_game_render(data);
			break;
		}
	}
	al_hold_bitmap_drawing(true);
	for(i = 0; i < DOT_MAX_PARTICLES; i++)
	{
		dot_particle_render(&app->particle[i], app->bitmap[DOT_BITMAP_PARTICLE]);
	}
	al_hold_bitmap_drawing(false);
}

static bool dot_load_bitmap(APP_INSTANCE * app, int bitmap, const char * fn)
{
	t3f_load_resource((void **)&app->bitmap[bitmap], T3F_RESOURCE_TYPE_BITMAP, fn, 0, 0, 0);
	if(!app->bitmap[bitmap])
	{
		printf("Failed to load image %d!\n", bitmap);
		return false;
	}
	return true;
}

/* initialize our app, load graphics, etc. */
bool app_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	int i;
	const char * val;

	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, 540, 960, 60.0, app_logic, app_render, T3F_DEFAULT, app))
	{
		printf("Error initializing T3F\n");
		return false;
	}

	/* load images */
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_RED, "data/graphics/ball_red.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_GREEN, "data/graphics/ball_green.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_BLUE, "data/graphics/ball_blue.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_PURPLE, "data/graphics/ball_purple.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_YELLOW, "data/graphics/ball_yellow.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_ORANGE, "data/graphics/ball_orange.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_BLACK, "data/graphics/ball_black.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_EYES, "data/graphics/ball_eyes.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_COMBO, "data/graphics/combo.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_PARTICLE, "data/graphics/particle.png"))
	{
		return false;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_LOGO, "data/graphics/logo.png"))
	{
		return false;
	}
	app->bitmap[DOT_BITMAP_SCRATCH] = al_create_bitmap(256, 256);
	if(!app->bitmap[DOT_BITMAP_SCRATCH])
	{
		printf("Failed to create effects scratch bitmap!\n");
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

	/* load fonts */
	app->font[DOT_FONT_16] = al_load_font("data/fonts/kongtext.ttf", 16, 0);
	if(!app->font[DOT_FONT_16])
	{
		printf("Failed to load font %d!\n", DOT_FONT_16);
		return false;
	}
	app->font[DOT_FONT_32] = al_load_font("data/fonts/kongtext.ttf", 32, 0);
	if(!app->font[DOT_FONT_32])
	{
		printf("Failed to load font %d!\n", DOT_FONT_32);
		return false;
	}

	/* build atlas */
	app->atlas = t3f_create_atlas(1024, 1024);
	if(app->atlas)
	{
		for(i = 0; i < DOT_BITMAP_ATLAS_BOUNDARY; i++)
		{
			if(app->bitmap[i])
			{
				t3f_add_bitmap_to_atlas(app->atlas, &app->bitmap[i], T3F_ATLAS_SPRITE);
			}
		}
	}

	/* load high score */
	val = al_get_config_value(t3f_config, "Game Data", "High Score");
	if(val)
	{
		app->game.high_score = atoi(val);
	}
	else
	{
		app->game.high_score = 0;
	}

	/* load user name */
	val = al_get_config_value(t3f_config, "Game Data", "User Name");
	if(val)
	{
		strcpy(app->user_name, val);
	}
	else
	{
		strcpy(app->user_name, "Anonymous");
	}

	app->upload_scores = false;
	val = al_get_config_value(t3f_config, "Game Data", "Upload Scores");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			app->upload_scores = true;
		}
	}

	app->music_enabled = false;
	val = al_get_config_value(t3f_config, "Game Data", "Music Enabled");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			app->music_enabled = true;
		}
	}

	if(!dot_intro_initialize(app))
	{
		printf("Failed to create menu!\n");
		return false;
	}

	/* change view focus so 3D effects look right */
	t3f_set_view_focus(t3f_current_view, DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2);

	app->state = 0;
	return true;
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
		printf("Error: could not initialize T3F!\n");
	}
	return 0;
}
