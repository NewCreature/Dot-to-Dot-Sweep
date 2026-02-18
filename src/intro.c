#include "t3f/t3f.h"
#include "t3f/android.h"
#include "instance.h"
#include "game.h"
#include "text.h"
#include "color.h"
#include "intro.h"
#include "leaderboard.h"
#include "mouse.h"

static void select_first_element(APP_INSTANCE * app)
{
	if(app->input_type == DOT_INPUT_CONTROLLER)
	{
		app->menu[app->current_menu]->hover_element = 0;
		if(!app->menu[app->current_menu]->element[app->menu[app->current_menu]->hover_element].proc)
		{
			t3f_select_next_gui_element(app->menu[app->current_menu]);
		}
	}
}

static void select_last_element(APP_INSTANCE * app)
{
	if(app->input_type == DOT_INPUT_CONTROLLER)
	{
		app->menu[app->current_menu]->hover_element = -1;
		t3f_select_previous_gui_element(app->menu[app->current_menu]);
	}
}

static void remember_element(APP_INSTANCE * app)
{
	if(app->input_type ==  DOT_INPUT_CONTROLLER)
	{
		app->previous_element = app->menu[app->current_menu]->hover_element;
	}
}

static void recall_element(APP_INSTANCE * app)
{
	if(app->input_type == DOT_INPUT_CONTROLLER)
	{
		app->menu[app->current_menu]->hover_element = app->previous_element;
	}
}

static void dot_update_first_run(void)
{
	al_set_config_value(t3f_user_data, "Game Data", "Setup Done", "true");
	t3f_save_user_data();
}

int dot_menu_proc_game_mode_easy(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->game_mode = 1;
	al_set_config_value(t3f_user_data, "Game Data", "Game Mode", "1");
	t3f_save_user_data();
	app->current_menu = DOT_MENU_MOUSE;

	return 1;
}

int dot_menu_proc_game_mode_normal(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->game_mode = 0;
	al_set_config_value(t3f_user_data, "Game Data", "Game Mode", "0");
	t3f_save_user_data();
	app->current_menu = DOT_MENU_MOUSE;

	return 1;
}

int dot_menu_proc_mouse_sensitivity_down(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->mouse_sensitivity > 0.0)
	{
		app->mouse_sensitivity -= 0.05;
	}
	if(app->mouse_sensitivity < 0.05)
	{
		app->mouse_sensitivity = 0.0;
	}

	return 1;
}

int dot_menu_proc_mouse_sensitivity_up(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->mouse_sensitivity < 2.0)
	{
		app->mouse_sensitivity += 0.05;
	}
	if(app->mouse_sensitivity > 2.0)
	{
		app->mouse_sensitivity = 2.0;
	}

	return 1;
}

int dot_menu_proc_mouse_sensitivity_default(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	
	app->mouse_sensitivity = 0.0;

	return 1;
}

int dot_menu_proc_mouse_sensitivity_ok(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char buf[32];

	sprintf(buf, "%1.2f", app->mouse_sensitivity);
	al_set_config_value(t3f_config, "App Config", "Mouse Sensitivity", buf);
	t3f_save_config();
	dot_update_first_run();
	app->current_menu = DOT_MENU_MAIN;
	recall_element(app);

	return 1;
}

int dot_menu_proc_play(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	dot_game_initialize(data, app->demo_file ? true : false, app->game_mode);

	return 1;
}

int dot_menu_proc_leaderboard(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;

	al_stop_timer(t3f_timer);
	if(app->upload_scores)
	{
		dot_upload_current_high_score(data);
	}
	dot_show_message(data, "Downloading leaderboard...");
	app->leaderboard = t3f_get_leaderboard("Game Data", app->game_mode == 0 ? "normal" : "easy", "none", 10, 0, false);
	if(app->leaderboard)
	{
		remember_element(app);
		app->leaderboard_spot = -1;
		val = al_get_config_value(t3f_user_data, "Game Data", app->game_mode == 0 ? "High Score" : "High Score Easy");
		if(val)
		{
			app->leaderboard_spot = dot_get_leaderboard_spot(app->leaderboard, app->user_name, atoi(val));
		}
		t3f_clear_touch_state();
		app->state = DOT_STATE_LEADERBOARD;
		app->current_menu = DOT_MENU_LEADERBOARD;
	}
	al_resume_timer(t3f_timer);
	return 1;
}

int dot_menu_proc_exit(void * data, int i, void * pp)
{
	t3f_exit();

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

int dot_menu_proc_music_yes(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->music_enabled = true;
	al_set_config_value(t3f_user_data, "Game Data", "Music Enabled", "true");
	if(t3f_get_music_state() != T3F_MUSIC_STATE_PLAYING)
	{
		t3f_play_music(DOT_MUSIC_TITLE);
	}
	app->current_menu = DOT_MENU_GAME_MODE;
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
	app->current_menu = DOT_MENU_GAME_MODE;
	return 1;
}

int dot_menu_proc_privacy(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	remember_element(app);
	t3f_clear_touch_state();
	app->current_menu = DOT_MENU_PRIVACY;
	app->state = DOT_STATE_PRIVACY;
	return 1;
}

void dot_menu_proc_profile_name_callback(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(strlen(app->user_name) <= 0)
	{
		strcpy(app->user_name, "Anonymous");
	}
	al_set_config_value(t3f_user_data, "Game Data", "User Name", app->user_name);
	t3f_save_user_data();
}

int dot_menu_proc_profile_name(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->desktop_mode)
	{
		dot_clear_input(&app->controller);
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
	if(strlen(app->user_name) > 0)
	{
		al_set_config_value(t3f_user_data, "Game Data", "User Name", "");
	}
	app->entering_name = false;
	return 1;
}

int dot_menu_proc_upload_yes(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;

	app->upload_scores = true;
	al_set_config_value(t3f_user_data, "Game Data", "Upload Scores", "true");
	val = t3f_get_steam_user_display_name();
	if(val)
	{
		al_set_config_value(t3f_user_data, "Game Data", "Steam User Display Name", val);
		strcpy(app->user_name, val);
		app->current_menu = DOT_MENU_MUSIC;
		select_first_element(app);
	}
	else
	{
		app->current_menu = DOT_MENU_PROFILE;
		select_first_element(app);
	}
	t3f_save_user_data();
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
	const char * val;

	app->current_menu = DOT_MENU_MAIN;
	recall_element(app);
	val = al_get_config_value(t3f_user_data, "Game Data", "User Name");
	if(val)
	{
		strcpy(app->user_name, val);
	}
	app->entering_name = false;

	return 1;
}

int dot_menu_proc_leaderboard_back(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3f_destroy_leaderboard(app->leaderboard);
	app->leaderboard = NULL;
	dot_intro_setup(data);
	t3f_clear_mouse_state();
	t3f_clear_touch_state();
	app->state = DOT_STATE_INTRO;
	app->current_menu = DOT_MENU_MAIN;
	recall_element(app);

	return 1;
}

int dot_menu_proc_privacy_back(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	dot_intro_setup(data);
	t3f_clear_mouse_state();
	t3f_clear_touch_state();
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
		t3f_destroy_leaderboard(app->leaderboard);
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

int dot_menu_proc_pause_resume(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	/* allow touch that resumes game to continue to be used to play */
	if(app->game.pause_state == DOT_GAME_STATE_START)
	{
		t3f_clear_touch_state();
	}
	else
	{
		/* move mouse to player position if playing with mouse */
		if(app->input_type == DOT_INPUT_MOUSE)
		{
			t3f_set_mouse_xy(app->game.player.ball.x, app->game.player.ball.y);
			app->mickey_ticks = 3;
		}
	}
	app->game.state = app->game.pause_state;

	return 1;
}

int dot_menu_proc_pause_quit(void * data, int i, void * pp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	dot_game_exit(data, true);
	dot_enable_mouse_cursor(true);
	app->state = DOT_STATE_INTRO;
	if(app->music_enabled)
	{
		t3f_play_music(DOT_MUSIC_TITLE);
	}

	return 1;
}

void dot_intro_center_menus(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	for(i = 0; i < DOT_MAX_MENUS; i++)
	{
		if(app->menu[i])
		{
			t3f_center_gui(app->menu[i], 0, app->menu_view->virtual_height);
		}
	}
}

bool dot_intro_process_menu(void * data, T3F_GUI * mp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	bool ret = false;

	t3f_select_input_view(app->menu_view);
	switch(app->input_type)
	{
		case DOT_INPUT_MOUSE:
		{
			t3f_select_hover_gui_element(mp, t3f_get_mouse_x(), t3f_get_mouse_y());
			if(t3f_mouse_button_pressed(0))
			{
				if(mp->hover_element >= 0)
				{
					t3f_activate_selected_gui_element(mp);
					ret = true;
				}
				t3f_use_mouse_button_press(0);
				t3f_use_touch_press(0);
			}
			break;
		}
		case DOT_INPUT_TOUCH:
		{
			if(t3f_touch_pressed(1))
			{
				t3f_select_hover_gui_element(mp, t3f_get_touch_x(1), t3f_get_touch_y(1));
				if(mp->hover_element >= 0)
				{
					t3f_activate_selected_gui_element(mp);
					ret = true;
				}
				mp->hover_element = -1;
				t3f_use_touch_press(1);
			}
			break;
		}
		case DOT_INPUT_CONTROLLER:
		{
			if(app->controller.axis_y < 0.0 && app->controller.axis_y_pressed)
			{
				t3f_select_previous_gui_element(mp);
				t3f_use_key_press(ALLEGRO_KEY_UP);
				t3f_use_key_press(ALLEGRO_KEY_W);
				app->controller.axis_y_pressed = false;
			}
			if(app->controller.axis_y > 0.0 && app->controller.axis_y_pressed)
			{
				t3f_select_next_gui_element(mp);
				t3f_use_key_press(ALLEGRO_KEY_DOWN);
				t3f_use_key_press(ALLEGRO_KEY_S);
				app->controller.axis_y_pressed = false;
			}
			if(mp->hover_element < 0)
			{
				t3f_select_previous_gui_element(mp);
			}
			if(app->controller.button)
			{
				t3f_activate_selected_gui_element(mp);
				app->controller.button = false;
				ret = true;
			}
			break;
		}
	}

	return ret;
}

bool dot_intro_initialize(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int pos_y = 0;

	t3f_set_gui_driver(NULL);

	/* create menus */
	app->menu[DOT_MENU_GAME_MODE] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_GAME_MODE])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_GAME_MODE], NULL, app->font[DOT_FONT_32], al_map_rgba_f(1.0, 1.0, 0.0, 1.0), "Difficulty", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_GAME_MODE], dot_menu_proc_game_mode_easy, app->font[DOT_FONT_32], t3f_color_white, "Easy", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_GAME_MODE], dot_menu_proc_game_mode_normal, app->font[DOT_FONT_32], t3f_color_white, "Normal", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_GAME_MODE], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_GAME_MODE], 2, -2);

	pos_y = 0;
	sprintf(app->mouse_menu_sensitivity_text, "-------");
	app->menu[DOT_MENU_MOUSE] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_MOUSE])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_MOUSE], NULL, app->font[DOT_FONT_32], al_map_rgba_f(1.0, 1.0, 0.0, 1.0), "Mouse/Touch", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	pos_y += 32;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MOUSE], NULL, app->font[DOT_FONT_32], al_map_rgba_f(1.0, 1.0, 0.0, 1.0), "Sensitivity", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MOUSE], NULL, app->font[DOT_FONT_32], t3f_color_white, app->mouse_menu_sensitivity_text, t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MOUSE], dot_menu_proc_mouse_sensitivity_down, app->font[DOT_FONT_32], t3f_color_white, "<", t3f_virtual_display_width / 2 - t3f_get_text_width(app->font[DOT_FONT_32], app->mouse_menu_sensitivity_text) / 2 - t3f_get_text_width(app->font[DOT_FONT_32], "<"), pos_y, T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MOUSE], dot_menu_proc_mouse_sensitivity_up, app->font[DOT_FONT_32], t3f_color_white, ">", t3f_virtual_display_width / 2 + t3f_get_text_width(app->font[DOT_FONT_32], app->mouse_menu_sensitivity_text) / 2, pos_y, T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MOUSE], dot_menu_proc_mouse_sensitivity_default, app->font[DOT_FONT_32], t3f_color_white, "Default", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MOUSE], dot_menu_proc_mouse_sensitivity_ok, app->font[DOT_FONT_32], t3f_color_white, "Okay", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_MOUSE], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_MOUSE], 2, -2);

	pos_y = 0;
	app->menu[DOT_MENU_MAIN] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_MAIN])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_exit, app->font[DOT_FONT_32], t3f_color_white, "Exit to OS", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_leaderboard, app->font[DOT_FONT_32], t3f_color_white, "Leaderboard", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_setup, app->font[DOT_FONT_32], t3f_color_white, "Setup", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_privacy, app->font[DOT_FONT_32], DOT_MENU_COLOR_ENABLED, "Privacy", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	pos_y += 64;
	t3f_add_gui_text_element(app->menu[DOT_MENU_MAIN], dot_menu_proc_play, app->font[DOT_FONT_32], t3f_color_white, "Play", t3f_virtual_display_width / 2, pos_y, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_MAIN], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_MAIN], 2, -2);

	app->menu[DOT_MENU_PRIVACY] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_PRIVACY])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_PRIVACY], dot_menu_proc_privacy_back, app->font[DOT_FONT_32], t3f_color_white, "Back", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_PRIVACY], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_PRIVACY], 2, -2);

	app->menu[DOT_MENU_LEADERBOARD] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_LEADERBOARD])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD], dot_menu_proc_leaderboard_back, app->font[DOT_FONT_32], t3f_color_white, "Back", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_LEADERBOARD], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_LEADERBOARD], 2, -2);

	app->menu[DOT_MENU_LEADERBOARD_2] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_LEADERBOARD_2])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD_2], NULL, app->font[DOT_FONT_32], al_map_rgba_f(1.0, 1.0, 0.0, 1.0), "Play Again?", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD_2], dot_menu_proc_play, app->font[DOT_FONT_32], t3f_color_white, "Yes", t3f_virtual_display_width / 2, 64, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_LEADERBOARD_2], dot_menu_proc_leaderboard_main_menu, app->font[DOT_FONT_32], t3f_color_white, "No", t3f_virtual_display_width / 2, 128, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_LEADERBOARD_2], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_LEADERBOARD_2], 2, -2);

	app->menu[DOT_MENU_UPLOAD_SCORES] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_UPLOAD_SCORES])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_UPLOAD_SCORES], NULL, app->font[DOT_FONT_32], al_map_rgba_f(1.0, 1.0, 0.0, 1.0), "Upload Scores?", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_UPLOAD_SCORES], dot_menu_proc_upload_yes, app->font[DOT_FONT_32], t3f_color_white, "Yes", t3f_virtual_display_width / 2, 64, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_UPLOAD_SCORES], dot_menu_proc_upload_no, app->font[DOT_FONT_32], t3f_color_white, "No", t3f_virtual_display_width / 2, 128, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_UPLOAD_SCORES], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_UPLOAD_SCORES], 2, -2);

	app->menu[DOT_MENU_PROFILE] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_PROFILE])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], NULL, app->font[DOT_FONT_32], al_map_rgba_f(1.0, 1.0, 0.0, 1.0), "User Name", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_name, app->font[DOT_FONT_32], t3f_color_white, app->user_name, t3f_virtual_display_width / 2, 64, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PROFILE], dot_menu_proc_profile_okay, app->font[DOT_FONT_32], t3f_color_white, "Okay", t3f_virtual_display_width / 2, 128, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_PROFILE], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_PROFILE], 2, -2);

	app->menu[DOT_MENU_MUSIC] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_MUSIC])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_MUSIC], NULL, app->font[DOT_FONT_32], al_map_rgba_f(1.0, 1.0, 0.0, 1.0), "Enable Music?", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW | T3F_GUI_ELEMENT_STATIC);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MUSIC], dot_menu_proc_music_yes, app->font[DOT_FONT_32], t3f_color_white, "Yes", t3f_virtual_display_width / 2, 64, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_MUSIC], dot_menu_proc_music_no, app->font[DOT_FONT_32], t3f_color_white, "No", t3f_virtual_display_width / 2, 128, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_MUSIC], -2, 2);
	t3f_set_gui_hover_lift(app->menu[DOT_MENU_MUSIC], 2, -2);

	app->menu[DOT_MENU_PAUSE] = t3f_create_gui(0, 0, app);
	if(!app->menu[DOT_MENU_PAUSE])
	{
		return false;
	}
	t3f_add_gui_text_element(app->menu[DOT_MENU_PAUSE], dot_menu_proc_pause_resume, app->font[DOT_FONT_32], t3f_color_white, "Resume", t3f_virtual_display_width / 2, 0, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_add_gui_text_element(app->menu[DOT_MENU_PAUSE], dot_menu_proc_pause_quit, app->font[DOT_FONT_32], t3f_color_white, "Quit", t3f_virtual_display_width / 2, 64, T3F_GUI_ELEMENT_CENTRE | T3F_GUI_ELEMENT_SHADOW);
	t3f_set_gui_shadow(app->menu[DOT_MENU_PAUSE], -2, 2);
  	t3f_set_gui_hover_lift(app->menu[DOT_MENU_PAUSE], 2, -2);
	
	dot_intro_center_menus(data);
	app->current_menu = DOT_MENU_MAIN;

	return true;
}

/* reset intro screen variables so we start at the title screen and the main menu */
void dot_intro_setup(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3f_clear_touch_state();
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
	const char * val;
	int r;
	bool upload_user_name = false;
	bool m = false;

	if(app->mouse_sensitivity <= 0.0)
	{
		strcpy(app->mouse_menu_sensitivity_text, "1:1");
	}
	else
	{
		sprintf(app->mouse_menu_sensitivity_text, "%1.2f", app->mouse_sensitivity);
	}
	if(app->entering_name)
	{
		r = dot_text_entry_logic();
		if(r != 0)
		{
			app->entering_name = false;
		}
		if(r == 1)
		{
			dot_menu_proc_profile_okay(app, 0, NULL);
		}
	}
	else
	{
		if(!app->tried_user_name_upload)
		{
			/* upload user name if we failed to previously */
			val = al_get_config_value(t3f_user_data, "Game Data", "User Name Uploaded");
			if(val && !strcmp(val, "false"))
			{
				upload_user_name = true;
			}
			app->tried_user_name_upload = true;
		}
		/* upload user name if it's been changed */
		val = al_get_config_value(t3f_user_data, "Game Data", "User Name");
		if(!val || strcmp(val, app->user_name))
		{
			upload_user_name = true;
		}
	}
	if(upload_user_name)
	{
		al_set_config_value(t3f_user_data, "Game Data", "User Name Uploaded", "false");
		al_set_config_value(t3f_user_data, "Game Data", "User Name", app->user_name);
		t3f_save_user_data();
		al_stop_timer(t3f_timer);
		dot_show_message(data, "Synchronizing User Key Data...");
		if(t3f_submit_leaderboard_user_name("Game Data"))
		{
			al_remove_config_key(t3f_user_data, "Game Data", "User Name Uploaded");
			t3f_save_user_data();
		}
		al_start_timer(t3f_timer);
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
		if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE) || t3f_key_pressed(ALLEGRO_KEY_BACK))
		{
			if(app->desktop_mode)
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
				if(app->current_menu == DOT_MENU_MAIN)
				{
					t3f_exit();
				}
				else
				{
					app->current_menu = DOT_MENU_MAIN;
				}
			}
			t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
			t3f_use_key_press(ALLEGRO_KEY_BACK);
		}
		else
		{
			dot_intro_process_menu(app, app->menu[app->current_menu]);
		}
	}
	else
	{
		if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE) || t3f_key_pressed(ALLEGRO_KEY_BACK))
		{
			t3f_exit();
			t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
			t3f_use_key_press(ALLEGRO_KEY_BACK);
		}
		else
		{
			if(app->touch_id >= 0 && t3f_touch_pressed(app->touch_id))
			{
				m = true;
				t3f_use_mouse_button_press(0);
				t3f_use_touch_press(app->touch_id);
			}
			if(app->controller.button)
			{
				m = true;
				app->controller.button = false;
			}
			if(m)
			{
				app->menu_showing = true;
			}
		}
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

	al_draw_filled_rectangle(0, DOT_GAME_PLAYFIELD_HEIGHT, 540 + 0.5, DOT_GAME_PLAYFIELD_HEIGHT + 80 + 0.5, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
	al_hold_bitmap_drawing(true);
	sprintf(buffer, "High Score");
	dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, shadow, t3f_virtual_display_width / 2, 440 + 40 - t3f_get_font_line_height(app->font[DOT_FONT_32]), DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, buffer);
	sprintf(buffer, "%d", app->game.high_score[app->game_mode]);
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

	t3f_select_view(t3f_default_view);
	al_clear_to_color(app->level_color[0]);
	t3f_select_view(app->main_view);
	al_hold_bitmap_drawing(true);
	dot_bg_objects_render(data);
	t3f_draw_bitmap(app->bitmap[DOT_BITMAP_BG], t3f_color_white, 0, 0, 0, 0);
	if(!app->desktop_mode || !app->menu_showing)
	{
		w = al_get_bitmap_width(app->bitmap[DOT_BITMAP_LOGO]->bitmap) / app->graphics_size_multiplier;
		h = al_get_bitmap_height(app->bitmap[DOT_BITMAP_LOGO]->bitmap) / app->graphics_size_multiplier;
		t3f_draw_scaled_bitmap(app->bitmap[DOT_BITMAP_LOGO], t3f_color_white, DOT_GAME_PLAYFIELD_WIDTH / 2 - w / 2 + app->logo_ox, DOT_GAME_PLAYFIELD_HEIGHT / 2 - h / 2, 0, w, h, 0);
		dot_credits_render(data, app->credits_ox);
		render_copyright_message(data, t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2 - t3f_get_text_width(app->font[DOT_FONT_14], T3F_APP_COPYRIGHT) / 2 + app->logo_ox, DOT_GAME_PLAYFIELD_HEIGHT - t3f_get_font_line_height(app->font[DOT_FONT_16]) * 2, DOT_SHADOW_OX, DOT_SHADOW_OY);
	}
	al_hold_bitmap_drawing(false);
	dot_intro_render_split(data);
	if(app->menu_showing)
	{
		t3f_select_view(app->menu_view);
		al_hold_bitmap_drawing(true);
		t3f_render_gui(app->menu[app->current_menu], 0);
		if(app->entering_name)
		{
			if((app->tick / 15) % 2)
			{
				dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), app->menu[DOT_MENU_PROFILE]->ox + app->menu[DOT_MENU_PROFILE]->element[1].ox + t3f_get_text_width((T3F_FONT *)app->menu[DOT_MENU_PROFILE]->element[1].resource, app->menu[DOT_MENU_PROFILE]->element[1].data) / 2, app->menu[DOT_MENU_PROFILE]->oy + app->menu[DOT_MENU_PROFILE]->element[1].oy, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, "_");
			}
		}
		al_hold_bitmap_drawing(false);
	}
}
