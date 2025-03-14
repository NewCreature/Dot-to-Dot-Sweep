#include "t3f/t3f.h"
#include "instance.h"
#include "intro.h"
#include "text.h"

static const char * privacy_text[] =
{
	"",
	"",
	"",
	"Information We Collect:",
	"",
	" We collect only enough",
	" information to process your",
	" request. With regards to this",
	" software, that entails storing",
	" any scores you submit to our",
	" leaderboards (includes your",
	" profile name and score",
	" information).",
	"",
	"",
	"What We Do with Your",
	"Information:",
	"",
	" Your leaderboard data is",
	" accessible to everyone, so it",
	" is advisable to not put",
	" personal information you don't",
	" want to share in your screen",
	" name.",
	NULL
};

void dot_privacy_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	bool m = false;
	int i;

	dot_bg_objects_logic(data, DOT_GAME_LEVEL_BASE_SPEED);
	if(app->desktop_mode)
	{
		for(i = 0; i < T3F_MAX_TOUCHES; i++)
		{
			if(t3f_touch_pressed(i))
			{
				m = true;
				t3f_use_touch_press(i);
			}
		}
		if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE) || t3f_key_pressed(ALLEGRO_KEY_BACK) || m || app->controller.button)
		{
			dot_menu_proc_privacy_back(data, 0, NULL);
			t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
			t3f_use_key_press(ALLEGRO_KEY_BACK);
			app->controller.button = false;
		}
	}
	else
	{
		if(!dot_intro_process_menu(app, app->menu[app->current_menu]))
		{
			if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE))
			{
				m = true;
				t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
			}
			if(t3f_key_pressed(ALLEGRO_KEY_BACK))
			{
				m = true;
				t3f_use_key_press(ALLEGRO_KEY_BACK);
			}
			if(m)
			{
				dot_menu_proc_privacy_back(data, 0, NULL);
				app->controller.button = false;
			}
		}
	}
	app->tick++;
}

void dot_privacy_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	t3f_select_view(t3f_default_view);
	al_clear_to_color(app->level_color[0]);
	t3f_select_view(app->main_view);
	al_hold_bitmap_drawing(true);
	dot_bg_objects_render(data);
	t3f_draw_bitmap(app->bitmap[DOT_BITMAP_BG], t3f_color_white, 0, 0, 0, 0);
	for(i = 0; i < 128; i++)
	{
		if(privacy_text[i])
		{
			dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 8, i * 16, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, privacy_text[i]);
		}
		else
		{
			break;
		}
	}
	al_hold_bitmap_drawing(false);
	dot_intro_render_split(data);
	if(!app->desktop_mode)
	{
		t3f_select_view(app->menu_view);
		al_hold_bitmap_drawing(true);
		t3f_render_gui(app->menu[app->current_menu], 0);
		al_hold_bitmap_drawing(false);
	}
}
