#include "t3f/t3f.h"
#include "t3f/resource.h"
#include "t3f/music.h"
#include "t3f/gui.h"
#include "instance.h"
#include "intro.h"
#include "game.h"

void dot_shadow_text(ALLEGRO_FONT * font, ALLEGRO_COLOR color, ALLEGRO_COLOR shadow, float x, float y, float sx, float sy, int flags, const char * text)
{
	al_draw_text(font, shadow, x + sx, y + sy, flags, text);
	al_draw_text(font, color, x, y, flags, text);
}

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

	app_touch_logic(data);
	switch(app->state)
	{
		case DOT_STATE_INTRO:
		{
			dot_intro_logic(data);
			break;
		}
		case DOT_STATE_GAME:
		{
			dot_game_logic(data);
			break;
		}
	}
}

void app_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(app->state)
	{
		case DOT_STATE_INTRO:
		{
			dot_intro_render(data);
			break;
		}
		case DOT_STATE_GAME:
		{
			dot_game_render(data);
			break;
		}
	}
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
	t3f_load_resource((void **)&app->bitmap[DOT_BITMAP_BALL_RED], T3F_RESOURCE_TYPE_BITMAP, "data/graphics/ball_red.png", 0, 0, 0);
	if(!app->bitmap[DOT_BITMAP_BALL_RED])
	{
		printf("Failed to load image %d!\n", DOT_BITMAP_BALL_RED);
		return false;
	}
	t3f_load_resource((void **)&app->bitmap[DOT_BITMAP_BALL_GREEN], T3F_RESOURCE_TYPE_BITMAP, "data/graphics/ball_green.png", 0, 0, 0);
	if(!app->bitmap[DOT_BITMAP_BALL_GREEN])
	{
		printf("Failed to load image %d!\n", DOT_BITMAP_BALL_GREEN);
		return false;
	}
	t3f_load_resource((void **)&app->bitmap[DOT_BITMAP_BALL_BLUE], T3F_RESOURCE_TYPE_BITMAP, "data/graphics/ball_blue.png", 0, 0, 0);
	if(!app->bitmap[DOT_BITMAP_BALL_BLUE])
	{
		printf("Failed to load image %d!\n", DOT_BITMAP_BALL_BLUE);
		return false;
	}
	t3f_load_resource((void **)&app->bitmap[DOT_BITMAP_BALL_PURPLE], T3F_RESOURCE_TYPE_BITMAP, "data/graphics/ball_purple.png", 0, 0, 0);
	if(!app->bitmap[DOT_BITMAP_BALL_PURPLE])
	{
		printf("Failed to load image %d!\n", DOT_BITMAP_BALL_PURPLE);
		return false;
	}
	t3f_load_resource((void **)&app->bitmap[DOT_BITMAP_BALL_YELLOW], T3F_RESOURCE_TYPE_BITMAP, "data/graphics/ball_yellow.png", 0, 0, 0);
	if(!app->bitmap[DOT_BITMAP_BALL_YELLOW])
	{
		printf("Failed to load image %d!\n", DOT_BITMAP_BALL_YELLOW);
		return false;
	}
	t3f_load_resource((void **)&app->bitmap[DOT_BITMAP_BALL_ORANGE], T3F_RESOURCE_TYPE_BITMAP, "data/graphics/ball_orange.png", 0, 0, 0);
	if(!app->bitmap[DOT_BITMAP_BALL_ORANGE])
	{
		printf("Failed to load image %d!\n", DOT_BITMAP_BALL_ORANGE);
		return false;
	}
	t3f_load_resource((void **)&app->bitmap[DOT_BITMAP_BALL_BLACK], T3F_RESOURCE_TYPE_BITMAP, "data/graphics/ball_black.png", 0, 0, 0);
	if(!app->bitmap[DOT_BITMAP_BALL_BLACK])
	{
		printf("Failed to load image %d!\n", DOT_BITMAP_BALL_BLACK);
		return false;
	}
	t3f_load_resource((void **)&app->bitmap[DOT_BITMAP_BALL_EYES], T3F_RESOURCE_TYPE_BITMAP, "data/graphics/ball_eyes.png", 0, 0, 0);
	if(!app->bitmap[DOT_BITMAP_BALL_EYES])
	{
		printf("Failed to load image %d!\n", DOT_BITMAP_BALL_EYES);
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

	printf("break 1\n");
	app->font = al_load_font("data/fonts/kongtext.ttf", 16, 0);
	if(!app->font)
	{
		printf("Failed to load font!\n");
		return false;
	}

	/* build atlas */
	app->atlas = t3f_create_atlas(1024, 1024);
	if(app->atlas)
	{
		for(i = 0; i < DOT_MAX_BITMAPS; i++)
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
