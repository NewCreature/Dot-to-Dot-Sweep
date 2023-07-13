#include "t3f/t3f.h"
#include "t3f/android.h"
#include "instance.h"
#include "game.h"
#include "text.h"
#include "color.h"
#include "intro.h"

static void select_first_element(APP_INSTANCE * app)
{
	app->menu[app->current_menu]->hover_element = 0;
	if(!app->menu[app->current_menu]->element[app->menu[app->current_menu]->hover_element].proc)
	{
		t3f_select_next_gui_element(app->menu[app->current_menu]);
	}
}

static void select_last_element(APP_INSTANCE * app)
{
	app->menu[app->current_menu]->hover_element = -1;
	t3f_select_previous_gui_element(app->menu[app->current_menu]);
}

static void remember_element(APP_INSTANCE * app)
{
	app->previous_element = app->menu[app->current_menu]->hover_element;
}

static void recall_element(APP_INSTANCE * app)
{
	app->menu[app->current_menu]->hover_element = app->previous_element;
}

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
	app->leaderboard = t3net_get_leaderboard(app->leaderboard_retrieve_url, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "normal", "none", 10, 0);
	if(app->leaderboard)
	{
		remember_element(app);
		app->leaderboard_spot = -1;
		app->state = DOT_STATE_LEADERBOARD;
		app->current_menu = DOT_MENU_LEADERBOARD;
	}
	al_resume_timer(t3f_timer);
	return 1;
}

int dot_menu_proc_exit(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3f_exit();
	t3f_touch[app->touch_id].active = false;
	t3f_mouse_button[0] = false;

	return 1;
}

int dot_menu_proc_setup(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	remember_element(app);
	app->current_menu = DOT_MENU_UPLOAD_SCORES;
	select_first_element(app);
	return 1;
}

static void dot_update_first_run(void)
{
	al_set_config_value(t3f_user_data, "Game Data", "Setup Done", "true");
	t3f_save_user_data();
}

int dot_menu_proc_music_yes(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->music_enabled = true;
	al_set_config_value(t3f_user_data, "Game Data", "Music Enabled", "true");
	if(t3f_get_music_state() != T3F_MUSIC_STATE_PLAYING)
	{
		t3f_play_music(DOT_MUSIC_TITLE);
	}
	app->current_menu = DOT_MENU_MAIN;
	recall_element(app);
	dot_update_first_run();
	return 1;
}

int dot_menu_proc_music_no(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->music_enabled = false;
	al_set_config_value(t3f_user_data, "Game Data", "Music Enabled", "false");
	if(t3f_get_music_state() == T3F_MUSIC_STATE_PLAYING)
	{
		al_stop_timer(t3f_timer);
		t3f_stop_music();
		al_resume_timer(t3f_timer);
	}
	app->current_menu = DOT_MENU_MAIN;
	recall_element(app);
	dot_update_first_run();
	return 1;
}

int dot_menu_proc_privacy(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	remember_element(app);
	app->current_menu = DOT_MENU_PRIVACY;
	app->state = DOT_STATE_PRIVACY;
	t3f_mouse_button[0] = false;
	return 1;
}

void dot_menu_proc_profile_name_callback(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(strlen(app->user_name) <= 0)
	{
		strcpy(app->user_name, "Anonymous");
	}
	al_set_config_value(t3f_user_data, "Game Data", "User Name Uploaded", "false");
	al_set_config_value(t3f_user_data, "Game Data", "User Name", app->user_name);
	t3f_save_user_data();
	if(t3net_update_leaderboard_user_name(app->leaderboard_set_user_name_url, app->user_key, app->user_name))
	{
		al_remove_config_key(t3f_user_data, "Game Data", "User Name Uploaded");
		t3f_save_user_data();
	}
}

int dot_menu_proc_profile_name(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->desktop_mode)
	{
		app->entering_name = true;
		dot_enter_text(app->user_name, 256);
		t3f_select_next_gui_element(app->menu[app->current_menu]);
	}
	else
	{
		t3f_open_edit_box("Enter Name", app->user_name, 256, "CapWords", dot_menu_proc_profile_name_callback, app);
	}
	select_first_element(app);
	return 1;
}

int dot_menu_proc_profile_okay(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->current_menu = DOT_MENU_MUSIC;
	select_first_element(app);
	return 1;
}

int dot_menu_proc_upload_yes(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->upload_scores = true;
	al_set_config_value(t3f_user_data, "Game Data", "Upload Scores", "true");
	t3f_save_user_data();
	app->current_menu = DOT_MENU_PROFILE;
	select_first_element(app);
	return 1;
}

int dot_menu_proc_upload_no(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->upload_scores = false;
	al_set_config_value(t3f_user_data, "Game Data", "Upload Scores", "false");
	t3f_save_user_data();
	app->current_menu = DOT_MENU_MUSIC;
	select_first_element(app);
	return 1;
}

int dot_menu_proc_profile_back(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->current_menu = DOT_MENU_MAIN;
	recall_element(app);

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
	recall_element(app);

	return 1;
}

int dot_menu_proc_privacy_back(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	dot_intro_setup(data);
	app->state = DOT_STATE_INTRO;
	app->current_menu = DOT_MENU_MAIN;
	recall_element(app);

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
	select_last_element(app);
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
	int pos_y = 0;

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
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_exit, "Exit to OS", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, pos_y, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_leaderboard, "Leaderboard", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, pos_y, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_setup, "Setup", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, pos_y, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_privacy, "Privacy", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, pos_y, DOT_MENU_COLOR_ENABLED, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_play, "Play", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, pos_y, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_MAIN], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_MAIN], -2, 2);
  t3f_set_gui_hover_lift(app->menu[DOT_MENU_MAIN], 2, -2);

	app->menu[DOT_MENU_PRIVACY] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_PRIVACY])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_PRIVACY], dot_menu_proc_privacy_back, "Back", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_PRIVACY], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_PRIVACY], -2, 2);
  t3f_set_gui_hover_lift(app->menu[DOT_MENU_PRIVACY], 2, -2);

	app->menu[DOT_MENU_LEADERBOARD] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_LEADERBOARD])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD], dot_menu_proc_leaderboard_back, "Back", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_LEADERBOARD], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_LEADERBOARD], -2, 2);
  t3f_set_gui_hover_lift(app->menu[DOT_MENU_LEADERBOARD], 2, -2);

	app->menu[DOT_MENU_LEADERBOARD_2] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_LEADERBOARD_2])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD_2], NULL, "Play Again?", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, al_map_rgba_f(1.0, 1.0, 0.0, 1.0), T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD_2], dot_menu_proc_play, "Yes", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD_2], dot_menu_proc_leaderboard_main_menu, "No", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_LEADERBOARD_2], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_LEADERBOARD_2], -2, 2);
  t3f_set_gui_hover_lift(app->menu[DOT_MENU_LEADERBOARD_2], 2, -2);

	app->menu[DOT_MENU_UPLOAD_SCORES] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_UPLOAD_SCORES])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_UPLOAD_SCORES], NULL, "Upload Scores?", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, al_map_rgba_f(1.0, 1.0, 0.0, 1.0), T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_UPLOAD_SCORES], dot_menu_proc_upload_yes, "Yes", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_UPLOAD_SCORES], dot_menu_proc_upload_no, "No", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_UPLOAD_SCORES], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_UPLOAD_SCORES], -2, 2);
  t3f_set_gui_hover_lift(app->menu[DOT_MENU_UPLOAD_SCORES], 2, -2);

	app->menu[DOT_MENU_PROFILE] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_PROFILE])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], NULL, "User Name", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, al_map_rgba_f(1.0, 1.0, 0.0, 1.0), T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_name, app->user_name, (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_okay, "Okay", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_PROFILE], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_PROFILE], -2, 2);
  t3f_set_gui_hover_lift(app->menu[DOT_MENU_PROFILE], 2, -2);

	app->menu[DOT_MENU_MUSIC] = t3f_create_gui(0, 0);
	if(!app->menu[DOT_MENU_MUSIC])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_MUSIC], NULL, "Enable Music?", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 0, al_map_rgba_f(1.0, 1.0, 0.0, 1.0), T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MUSIC], dot_menu_proc_music_yes, "Yes", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 64, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MUSIC], dot_menu_proc_music_no, "No", (void **)&app->font[DOT_FONT_32], t3f_virtual_display_width / 2, 128, t3f_color_white, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_center_gui(app->menu[DOT_MENU_MUSIC], top, bottom);
	t3f_set_gui_shadow(app->menu[DOT_MENU_MUSIC], -2, 2);
  t3f_set_gui_hover_lift(app->menu[DOT_MENU_MUSIC], 2, -2);

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
			al_set_config_value(t3f_user_data, "Game Data", "User Name Uploaded", "false");
			al_set_config_value(t3f_user_data, "Game Data", "User Name", app->user_name);
			t3f_save_user_data();
			if(t3net_update_leaderboard_user_name(app->leaderboard_set_user_name_url, app->user_key, app->user_name))
			{
				al_remove_config_key(t3f_user_data, "Game Data", "User Name Uploaded");
				t3f_save_user_data();
			}
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
	if(app->menu_showing)
	{
		t3f_process_gui(app->menu[app->current_menu], app);
	}
	if(!app->menu_showing)
	{
		if(t3f_mouse_button[0] || app->controller.button)
		{
			app->menu_showing = true;
			t3f_mouse_button[0] = false;
			app->controller.button_blocked = true;
		}
	}
	else
	{
		if(!app->entering_name)
		{
			if(app->using_controller)
			{
				if(app->menu[app->current_menu]->hover_element < 0)
				{
					t3f_select_previous_gui_element(app->menu[app->current_menu]);
				}
			}
			if(app->controller.axis_y < 0.0)
			{
				t3f_select_previous_gui_element(app->menu[app->current_menu]);
				t3f_key[ALLEGRO_KEY_UP] = 0;
				app->controller.axes_blocked = true;
			}
			if(app->controller.axis_y > 0.0)
			{
				t3f_select_next_gui_element(app->menu[app->current_menu]);
				t3f_key[ALLEGRO_KEY_DOWN] = 0;
				app->controller.axes_blocked = true;
			}
			if(app->controller.button)
			{
				t3f_activate_selected_gui_element(app->menu[app->current_menu], app);
				app->controller.button_blocked = true;
			}
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
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, t3f_virtual_display_width / 2, 440 + 40 - t3f_get_font_line_height(app->font[DOT_FONT_32]), DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, buffer);
	sprintf(buffer, "%d", app->game.high_score);
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, t3f_virtual_display_width / 2, 440 + 40, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, buffer);
	al_hold_bitmap_drawing(false);
	al_hold_bitmap_drawing(held);
}

static void render_copyright_message(void * data, ALLEGRO_COLOR color, ALLEGRO_COLOR shadow, float x, float y, float sx, float sy)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3f_draw_text(app->font[DOT_FONT_14], shadow, x + sx, y + sy, 0, 0, T3F_APP_COPYRIGHT);
	t3f_draw_text(app->font[DOT_FONT_14], color, x, y, 0, 0, T3F_APP_COPYRIGHT);
}

void dot_intro_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int w, h;

	al_clear_to_color(app->level_color[0]);
	al_hold_bitmap_drawing(true);
	dot_bg_objects_render(data);
	al_draw_bitmap(app->bitmap[DOT_BITMAP_BG], 0, 0, 0);
	if(!app->desktop_mode || !app->menu_showing)
	{
		w = al_get_bitmap_width(app->bitmap[DOT_BITMAP_LOGO]) / app->graphics_size_multiplier;
		h = al_get_bitmap_height(app->bitmap[DOT_BITMAP_LOGO]) / app->graphics_size_multiplier;
		al_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_LOGO], 0, 0, al_get_bitmap_width(app->bitmap[DOT_BITMAP_LOGO]), al_get_bitmap_height(app->bitmap[DOT_BITMAP_LOGO]), DOT_GAME_PLAYFIELD_WIDTH / 2 - w / 2 + app->logo_ox, DOT_GAME_PLAYFIELD_HEIGHT / 2 - h / 2, w, h, 0);
		dot_credits_render(data, app->credits_ox);
		render_copyright_message(data, t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2 - t3f_get_text_width(app->font[DOT_FONT_14], T3F_APP_COPYRIGHT) / 2 + app->logo_ox, DOT_GAME_PLAYFIELD_HEIGHT - t3f_get_font_line_height(app->font[DOT_FONT_16]) * 2, DOT_SHADOW_OX, DOT_SHADOW_OY);
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
				dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), app->menu[DOT_MENU_PROFILE]->ox + app->menu[DOT_MENU_PROFILE]->element[1].ox + t3f_get_text_width(*app->menu[DOT_MENU_PROFILE]->element[1].resource, app->menu[DOT_MENU_PROFILE]->element[1].data) / 2, app->menu[DOT_MENU_PROFILE]->oy + app->menu[DOT_MENU_PROFILE]->element[1].oy, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, "_");
			}
		}
		al_hold_bitmap_drawing(false);
	}
}
