#include "t3f/t3f.h"
#include "instance.h"
#include "game.h"

void dot_shadow_text(ALLEGRO_FONT * font, ALLEGRO_COLOR color, ALLEGRO_COLOR shadow, float x, float y, float sx, float sy, int flags, const char * text);

int dot_menu_proc_play(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	srand(time(0));
	dot_game_initialize(data);
	t3f_touch[app->touch_id].active = false;

	return 1;
}

int dot_menu_proc_leaderboards(void * data, int i, void * pp)
{
	return 1;
}

int dot_menu_proc_profile(void * data, int i, void * pp)
{
	return 1;
}

int dot_menu_proc_music(void * data, int i, void * pp)
{
	return 1;
}

bool dot_intro_initialize(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	/* create menu */
	t3f_set_gui_driver(NULL);
	app->menu = t3f_create_gui(0, 0);
	if(!app->menu)
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu, dot_menu_proc_play, "Leaderboards", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu, dot_menu_proc_play, "Profile", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu, dot_menu_proc_play, "Music", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu, dot_menu_proc_play, "Play", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 192, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu, (t3f_virtual_display_height / 2 - DOT_GAME_PLAYFIELD_HEIGHT) / 2 + t3f_virtual_display_height / 2, t3f_virtual_display_height);

	return true;
}

void dot_intro_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->tick++;
	t3f_process_gui(app->menu, app);
	if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
	{
		t3f_exit();
	}
}

void dot_intro_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	int i;

	al_clear_to_color(al_map_rgb_f(0.5, 0.5, 0.5));
	al_hold_bitmap_drawing(true);
	for(i = 0; i < 540 / 16; i++)
	{
		al_draw_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + i % 6], i * 16 + 6, sin((float)(i * 2 + app->tick) / 10.0) * 32 + 64, 0);
		al_draw_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + i % 6], i * 16 + 6, cos((float)(i * 2 + app->tick) / 10.0) * 32 + DOT_GAME_PLAYFIELD_HEIGHT - 64 - 16 - 1, 0);
	}
	al_hold_bitmap_drawing(false);
	dot_game_render_hud(data);
	al_hold_bitmap_drawing(true);
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2 - 16, 4, 4, ALLEGRO_ALIGN_CENTRE, "Dot to Dot Sweep");
	t3f_render_gui(app->menu);
//	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), DOT_GAME_PLAYFIELD_WIDTH / 2, t3f_virtual_display_height / 2 + t3f_virtual_display_height / 4 - 16, 4, 4, ALLEGRO_ALIGN_CENTRE, "Play");
	al_hold_bitmap_drawing(false);
}
