#include "t3f/t3f.h"
#include "t3f/android.h"
#include "instance.h"
#include "game.h"
#include "text.h"
#include "color.h"
#include "intro.h"

int dot_menu_proc_play(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	dot_game_initialize(data, app->demo_file ? true : false);
	t3f_touch[app->touch_id].active = false;

	return 1;
}

int dot_menu_proc_leaderboard(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	al_stop_timer(t3f_timer);
	app->leaderboard = t3net_get_leaderboard(DOT_LEADERBOARD_RETRIEVE_URL, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "normal", "none", 20, 0);
	if(app->leaderboard)
	{
		app->state = DOT_STATE_LEADERBOARD;
	}
	al_resume_timer(t3f_timer);
	return 1;
}

int dot_menu_proc_profile(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->current_menu = DOT_MENU_PROFILE;
	return 1;
}

int dot_menu_proc_music(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	T3F_GUI * gui = (T3F_GUI *)pp;

	app->music_enabled = !app->music_enabled;
	if(app->music_enabled)
	{
		gui->element[i].color = DOT_MENU_COLOR_ENABLED;
		al_set_config_value(t3f_config, "Game Data", "Music Enabled", "true");
	}
	else
	{
		gui->element[i].color = DOT_MENU_COLOR_DISABLED;
		al_set_config_value(t3f_config, "Game Data", "Music Enabled", "false");
	}
	return 1;
}

int dot_menu_proc_credits(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->intro_state = DOT_INTRO_STATE_CREDITS;
	return 1;
}

void dot_menu_proc_profile_name_callback(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(strlen(app->user_name) <= 0)
	{
		strcpy(app->user_name, "Anonymous");
	}
	al_set_config_value(t3f_config, "Game Data", "User Name", app->user_name);
}

int dot_menu_proc_profile_name(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3f_open_edit_box("Enter Name", app->user_name, 256, "CapWords", dot_menu_proc_profile_name_callback, app);
	return 1;
}

int dot_menu_proc_profile_upload(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	T3F_GUI * gui = (T3F_GUI *)pp;

	app->upload_scores = !app->upload_scores;
	if(app->upload_scores)
	{
		gui->element[i].color = DOT_MENU_COLOR_ENABLED;
		al_set_config_value(t3f_config, "Game Data", "Upload Scores", "true");
	}
	else
	{
		gui->element[i].color = DOT_MENU_COLOR_DISABLED;
		al_set_config_value(t3f_config, "Game Data", "Upload Scores", "false");
	}
	return 1;
}

bool dot_intro_initialize(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3f_set_gui_driver(NULL);

	/* create menus */
	app->menu[DOT_MENU_MAIN] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_MAIN])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_leaderboard, "Leaderboard", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_profile, "Profile", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_music, "Music", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, app->music_enabled ? DOT_MENU_COLOR_ENABLED : DOT_MENU_COLOR_DISABLED, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_play, "Play", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 192, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_MAIN], (t3f_virtual_display_height / 2 - DOT_GAME_PLAYFIELD_HEIGHT) / 2 + t3f_virtual_display_height / 2, t3f_virtual_display_height);
	t3f_set_gui_shadow(app->menu[DOT_MENU_MAIN], -2, 2);

	app->menu[DOT_MENU_PROFILE] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_PROFILE])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], NULL, "User Name", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_name, app->user_name, app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_upload, "Upload Scores", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, app->upload_scores ? DOT_MENU_COLOR_ENABLED : DOT_MENU_COLOR_DISABLED, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_PROFILE], (t3f_virtual_display_height / 2 - DOT_GAME_PLAYFIELD_HEIGHT) / 2 + t3f_virtual_display_height / 2, t3f_virtual_display_height);
	t3f_set_gui_shadow(app->menu[DOT_MENU_PROFILE], -2, 2);

	app->current_menu = DOT_MENU_MAIN;

	return true;
}

void dot_intro_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	dot_bg_objects_logic(data, DOT_GAME_LEVEL_BASE_SPEED);
	switch(app->intro_state)
	{
		case DOT_INTRO_STATE_LOGO:
		{
			if(app->tick >= 900)
			{
				app->intro_state = DOT_INTRO_STATE_LOGO_OUT;
			}
			break;
		}
		case DOT_INTRO_STATE_LOGO_OUT:
		{
			app->logo_ox -= 4.0;
			app->credits_ox -= 4.0;
			if(app->logo_ox < -t3f_virtual_display_width)
			{
				app->credits.state = DOT_CREDITS_STATE_IN;
				app->intro_state = DOT_INTRO_STATE_CREDITS;
			}
			break;
		}
		case DOT_INTRO_STATE_CREDITS:
		{
			dot_credits_logic(&app->credits);
			if(app->credits.current_credit >= app->credits.credits)
			{
				app->credits.state = DOT_CREDITS_STATE_WAIT;
				app->intro_state = DOT_INTRO_STATE_CREDITS_OUT;
			}
			break;
		}
		case DOT_INTRO_STATE_CREDITS_OUT:
		{
			app->logo_ox += 4.0;
			app->credits_ox += 4.0;
			if(app->logo_ox >= 0.0)
			{
				app->credits.current_credit = 0;
				app->tick = 0;
				app->intro_state = DOT_INTRO_STATE_LOGO;
			}
			break;
		}
	}
	app->tick++;
	t3f_process_gui(app->menu[app->current_menu], app);
	if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
	{
		if(app->current_menu == DOT_MENU_MAIN)
		{
			t3f_exit();
		}
		else
		{
			app->current_menu = DOT_MENU_MAIN;
		}
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
		t3f_key[ALLEGRO_KEY_BACK] = 0;
	}
}

void dot_intro_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	al_clear_to_color(dot_darken_color(DOT_GAME_BG_COLOR, 0.75));
	al_hold_bitmap_drawing(true);
	dot_bg_objects_render(data);
	al_draw_bitmap(app->bitmap[DOT_BITMAP_BG], 0, 0, 0);
	al_draw_bitmap(app->bitmap[DOT_BITMAP_LOGO], DOT_GAME_PLAYFIELD_WIDTH / 2 - al_get_bitmap_width(app->bitmap[DOT_BITMAP_LOGO]) / 2 + app->logo_ox, DOT_GAME_PLAYFIELD_HEIGHT / 2 - al_get_bitmap_height(app->bitmap[DOT_BITMAP_LOGO]) / 2, 0);
	dot_credits_render(data, app->credits_ox);
	al_hold_bitmap_drawing(false);
	dot_game_render_hud(data);
	al_hold_bitmap_drawing(true);
	t3f_render_gui(app->menu[app->current_menu]);
	al_hold_bitmap_drawing(false);
}
