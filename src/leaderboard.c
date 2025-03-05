#include "t3f/t3f.h"
#include "instance.h"
#include "defines.h"
#include "text.h"
#include "color.h"
#include "intro.h"

unsigned long dot_leaderboard_obfuscate_score(unsigned long score)
{
  return score * DOT_LEADERBOARD_FACTOR + '2' + 'd' + 's';
}

unsigned long dot_leaderboard_unobfuscate_score(unsigned long score)
{
  return (score - 's' - 'd' - '2') / DOT_LEADERBOARD_FACTOR;
}

bool dot_verify_leaderboard_score(unsigned long score)
{
  return !((score - 's' - 'd' - '2') % DOT_LEADERBOARD_FACTOR);
}

/* Get user key from local storage or grab a new one. If we are able to get a
   valid key with either method, cache it in APP_INSTANCE->user_key and return
   true. Otherwise return false. */
bool dot_get_leaderboard_user_key(void * data)
{
  APP_INSTANCE * app = (APP_INSTANCE *)data;
  const char * val;
	char * new_val;
  bool ret = false;

  strcpy(app->user_key, "");
  val = al_get_config_value(t3f_user_data, "Game Data", "User Key");
  if(!val)
  {
    dot_show_message(data, "Retrieving User Key...");
    new_val = t3net_get_new_leaderboard_user_key(app->leaderboard_get_user_key_url, app->user_name);
    if(new_val)
    {
      if(strlen(new_val) < 128)
      {
        strcpy(app->user_key, new_val);
        al_set_config_value(t3f_user_data, "Game Data", "User Key", new_val);
        t3f_save_user_data();
        ret = true;
      }
      free(new_val);
      return ret;
    }
  }
  else
  {
    if(strlen(val) < 128)
    {
      strcpy(app->user_key, val);
      return true;
    }
  }
  return false;
}

void dot_upload_current_high_score(void * data)
{
  APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;
	const char * val2;
  unsigned long score;

  if(dot_get_leaderboard_user_key(data))
  {
    val = al_get_config_value(t3f_user_data, "Game Data", "High Score");
    if(val)
    {
      val2 = al_get_config_value(t3f_user_data, "Game Data", "High Score Level");
      if(val2)
      {
        score = atoi(val);
        if(dot_verify_leaderboard_score(score))
        {
          if(t3net_upload_score(app->leaderboard_submit_url, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "normal", "none", app->user_key, score, val2))
          {
            al_remove_config_key(t3f_user_data, "Game Data", "Score Uploaded");
          }
        }
        else
        {
          al_remove_config_key(t3f_user_data, "Game Data", "Score Uploaded");
        }
      }
    }

    val = al_get_config_value(t3f_user_data, "Game Data", "High Score Easy");
    if(val)
    {
      val2 = al_get_config_value(t3f_user_data, "Game Data", "High Score Level Easy");
      if(val2)
      {
        score = atoi(val);
        if(dot_verify_leaderboard_score(score))
        {
          if(t3net_upload_score(app->leaderboard_submit_url, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "easy", "none", app->user_key, score, val2))
          {
            al_remove_config_key(t3f_user_data, "Game Data", "Easy Score Uploaded");
          }
        }
        else
        {
          al_remove_config_key(t3f_user_data, "Game Data", "Easy Score Uploaded");
        }
      }
    }
  }
}

int dot_get_leaderboard_spot(T3NET_LEADERBOARD * lp, const char * name, unsigned long score)
{
	int i;

	for(i = 0; i < lp->entries; i++)
	{
		if(!strcmp(lp->entry[i]-> name, name) && lp->entry[i]->score == score)
		{
			return i;
		}
	}
	return -1;
}

void dot_leaderboard_logic(void * data)
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
    if(app->controller.button)
    {
      m = true;
      app->controller.button = false;
    }
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
    if(m && app->current_menu == DOT_MENU_LEADERBOARD_2)
    {
      dot_menu_proc_leaderboard_back(data, 0, NULL);
      app->current_menu = DOT_MENU_LEADERBOARD_2;
    }
    else if(m)
    {
      dot_menu_proc_leaderboard_back(data, 0, NULL);
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
      if(m && app->current_menu == DOT_MENU_LEADERBOARD_2)
      {
        dot_menu_proc_leaderboard_back(data, 0, NULL);
        app->current_menu = DOT_MENU_LEADERBOARD_2;
      }
      else if(m)
      {
        dot_menu_proc_leaderboard_back(data, 0, NULL);
      }
    }
  }
  app->tick++;
}

void dot_leaderboard_render(void * data)
{
  APP_INSTANCE * app = (APP_INSTANCE *)data;
  int i;
  char buf[256] = {0};
  ALLEGRO_COLOR text_color;

  t3f_select_view(t3f_default_view);
  al_clear_to_color(app->level_color[0]);
  t3f_select_view(app->main_view);
  al_hold_bitmap_drawing(true);
  dot_bg_objects_render(data);
  t3f_draw_bitmap(app->bitmap[DOT_BITMAP_BG], t3f_color_white, 0, 0, 0, 0);
  dot_shadow_text(app->font[DOT_FONT_32], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, 32, DOT_SHADOW_OX * 2, DOT_SHADOW_OY * 2, T3F_FONT_ALIGN_CENTER, "Leaderboard");
  for(i = 0; i < app->leaderboard->entries; i++)
  {
    if(i == app->leaderboard_spot && (app->tick / 6) % 2)
    {
      text_color = al_map_rgba_f(1.0, 1.0, 0.0, 1.0);
    }
    else
    {
      text_color = t3f_color_white;
    }
    sprintf(buf, " %s", app->leaderboard->entry[i]->name);
    dot_shadow_text(app->font[DOT_FONT_16], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 4, 4 + (i + 3) * 32, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, buf);
    sprintf(buf, "%lu ", dot_leaderboard_unobfuscate_score(app->leaderboard->entry[i]->score));
    dot_shadow_text(app->font[DOT_FONT_16], text_color, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width - 4, 4 + (i + 3) * 32, DOT_SHADOW_OX, DOT_SHADOW_OY, T3F_FONT_ALIGN_RIGHT, buf);
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
