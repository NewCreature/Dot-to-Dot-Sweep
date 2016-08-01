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
	t3f_mouse_button[0] = false;

	return 1;
}

int dot_menu_proc_leaderboard(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	al_stop_timer(t3f_timer);
	dot_show_message(data, "Downloading leaderboard...");
	app->leaderboard = t3net_get_leaderboard(DOT_LEADERBOARD_RETRIEVE_URL, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "normal", "none", 10, 0);
	if(app->leaderboard)
	{
		app->leaderboard_spot = -1;
		app->state = DOT_STATE_LEADERBOARD;
		app->current_menu = DOT_MENU_LEADERBOARD;
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
		t3f_play_music(DOT_MUSIC_TITLE);
	}
	else
	{
		gui->element[i].color = DOT_MENU_COLOR_DISABLED;
		al_set_config_value(t3f_config, "Game Data", "Music Enabled", "false");
		al_stop_timer(t3f_timer);
		t3f_stop_music();
		al_resume_timer(t3f_timer);
	}
	return 1;
}

int dot_menu_proc_privacy(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->current_menu = DOT_MENU_PRIVACY;
	app->state = DOT_STATE_PRIVACY;
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

	if(app->desktop_mode)
	{
		app->entering_name = true;
		dot_enter_text(app->user_name, 256);
	}
	else
	{
		t3f_open_edit_box("Enter Name", app->user_name, 256, "CapWords", dot_menu_proc_profile_name_callback, app);
	}
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

int dot_menu_proc_profile_back(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->current_menu = DOT_MENU_MAIN;

	return 1;
}

int dot_menu_proc_leaderboard_back(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3net_destroy_leaderboard(app->leaderboard);
	app->leaderboard = NULL;
	dot_intro_setup(data);
	app->state = DOT_STATE_INTRO;
	app->current_menu = DOT_MENU_MAIN;

	return 1;
}

int dot_menu_proc_privacy_back(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	dot_intro_setup(data);
	app->state = DOT_STATE_INTRO;
	app->current_menu = DOT_MENU_MAIN;

	return 1;
}

int dot_menu_proc_leaderboard_main_menu(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->leaderboard)
	{
		t3net_destroy_leaderboard(app->leaderboard);
		app->leaderboard = NULL;
	}
	dot_intro_setup(data);
	app->state = DOT_STATE_INTRO;
	app->current_menu = DOT_MENU_MAIN;
	if(app->music_enabled)
	{
		t3f_play_music(DOT_MUSIC_TITLE);
	}

	return 1;
}

bool dot_intro_initialize(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	float top, bottom;

	t3f_set_gui_driver(NULL);

	if(app->desktop_mode)
	{
		top = 0;
		bottom = DOT_GAME_PLAYFIELD_HEIGHT;
	}
	else
	{
		top = (t3f_virtual_display_height / 2 - DOT_GAME_PLAYFIELD_HEIGHT) / 2 + t3f_virtual_display_height / 2;
		bottom = t3f_virtual_display_height;
	}

	/* create menus */
	app->menu[DOT_MENU_MAIN] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_MAIN])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_leaderboard, "Leaderboard", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_profile, "Profile", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_music, "Music", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, app->music_enabled ? DOT_MENU_COLOR_ENABLED : DOT_MENU_COLOR_DISABLED, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_privacy, "Privacy", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 192, DOT_MENU_COLOR_ENABLED, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_play, "Play", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 256, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_MAIN], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_MAIN], -2, 2);

	app->menu[DOT_MENU_PROFILE] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_PROFILE])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], NULL, "User Name", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_name, app->user_name, app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_upload, "Upload Scores", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, app->upload_scores ? DOT_MENU_COLOR_ENABLED : DOT_MENU_COLOR_DISABLED, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_back, "Back", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 192, DOT_MENU_COLOR_ENABLED, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_PROFILE], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_PROFILE], -2, 2);

	app->menu[DOT_MENU_PRIVACY] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_PRIVACY])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_PRIVACY], dot_menu_proc_privacy_back, "Back", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_PRIVACY], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_PRIVACY], -2, 2);

	app->menu[DOT_MENU_LEADERBOARD] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_LEADERBOARD])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD], dot_menu_proc_leaderboard_back, "Back", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_LEADERBOARD], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_LEADERBOARD], -2, 2);

	app->menu[DOT_MENU_LEADERBOARD_2] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_LEADERBOARD_2])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD_2], dot_menu_proc_play, "Play Again", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD_2], dot_menu_proc_leaderboard_main_menu, "Main Menu", app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_LEADERBOARD_2], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_LEADERBOARD_2], -2, 2);

	app->current_menu = DOT_MENU_MAIN;

	return true;
}

/* reset intro screen variables so we start at the title screen and the main menu */
void dot_intro_setup(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->logo_ox = 0;
	app->intro_state = DOT_INTRO_STATE_LOGO;
	app->credits_ox = t3f_virtual_display_width;
	app->credits.ox = -t3f_virtual_display_width;
	app->credits.current_credit = 0;
	app->credits.state = DOT_CREDITS_STATE_WAIT;
	app->current_menu = DOT_MENU_MAIN;
	app->tick = 0;
}

void dot_intro_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int r;

	if(app->entering_name)
	{
		r = dot_text_entry_logic();
		if(r != 0)
		{
			app->entering_name = false;
		}
		if(r == 1)
		{
			al_set_config_value(t3f_config, "Game Data", "User Name", app->user_name);
		}
	}
	dot_game_emo_logic(data);
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
	if(!app->entering_name && app->menu_showing)
	{
		t3f_process_gui(app->menu[app->current_menu], app);
	}
	if(!app->menu_showing)
	{
		if(t3f_mouse_button[0])
		{
			app->menu_showing = true;
			t3f_mouse_button[0] = false;
		}
	}
	if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
	{
		if(app->desktop_mode)
		{
			if(app->menu_showing)
			{
				if(app->current_menu == DOT_MENU_MAIN)
				{
					dot_intro_setup(data);
					app->menu_showing = false;
				}
				else
				{
					if(app->current_menu == DOT_MENU_LEADERBOARD_2)
					{
						t3f_play_music(DOT_MUSIC_TITLE);
					}
					app->current_menu = DOT_MENU_MAIN;
				}
			}
			else
			{
				t3f_exit();
			}
		}
		else
		{
			if(app->current_menu == DOT_MENU_MAIN)
			{
				t3f_exit();
			}
			else
			{
				app->current_menu = DOT_MENU_MAIN;
			}
		}
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
		t3f_key[ALLEGRO_KEY_BACK] = 0;
	}
}

void dot_intro_render_split(void * data)
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
	sprintf(buffer, "High Score");
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, t3f_virtual_display_width / 2, 440 + 40 - al_get_font_line_height(app->font[DOT_FONT_32]), DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, ALLEGRO_ALIGN_CENTRE, buffer);
	sprintf(buffer, "%d", app->game.high_score);
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, t3f_virtual_display_width / 2, 440 + 40, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, ALLEGRO_ALIGN_CENTRE, buffer);
	al_hold_bitmap_drawing(false);
	al_hold_bitmap_drawing(held);
}

void dot_intro_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	al_clear_to_color(app->level_color[0]);
	al_hold_bitmap_drawing(true);
	dot_bg_objects_render(data);
	al_draw_bitmap(app->bitmap[DOT_BITMAP_BG], 0, 0, 0);
	if(!app->desktop_mode || !app->menu_showing)
	{
		al_draw_bitmap(app->bitmap[DOT_BITMAP_LOGO], DOT_GAME_PLAYFIELD_WIDTH / 2 - al_get_bitmap_width(app->bitmap[DOT_BITMAP_LOGO]) / 2 + app->logo_ox, DOT_GAME_PLAYFIELD_HEIGHT / 2 - al_get_bitmap_height(app->bitmap[DOT_BITMAP_LOGO]) / 2, 0);
		dot_credits_render(data, app->credits_ox);
		dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2 + app->logo_ox, DOT_GAME_PLAYFIELD_HEIGHT - al_get_font_line_height(app->font[DOT_FONT_16]) * 2, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_CENTRE, "Copyright (c) 2016 T^3 Software.");
	}
	al_hold_bitmap_drawing(false);
	dot_intro_render_split(data);
	if(app->menu_showing)
	{
		al_hold_bitmap_drawing(true);
		t3f_render_gui(app->menu[app->current_menu]);
		if(app->entering_name)
		{
			if((app->tick / 15) % 2)
			{
				dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), app->menu[DOT_MENU_PROFILE]->ox + app->menu[DOT_MENU_PROFILE]->element[1].ox + al_get_text_width(app->menu[DOT_MENU_PROFILE]->element[1].aux_data, app->menu[DOT_MENU_PROFILE]->element[1].data) / 2, app->menu[DOT_MENU_PROFILE]->oy + app->menu[DOT_MENU_PROFILE]->element[1].oy, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, "_");
			}
		}
		al_hold_bitmap_drawing(false);
	}
}
