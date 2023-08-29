#include "t3f/t3f.h"
#include "instance.h"
#include "text.h"
#include "color.h"
#include "intro.h"

unsigned long dot_leaderboard_obfuscate_score(unsigned long score)
{
  return score * 'd' + '2' + 'd' + 's';
}

unsigned long dot_leaderboard_unobfuscate_score(unsigned long score)
{
  return (score - 's' - 'd' - '2') / 'd';
}

void dot_upload_current_high_score(void * data)
{
  APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;
	const char * val2;

  val = al_get_config_value(t3f_user_data, "Game Data", "High Score");
  if(val)
  {
    val2 = al_get_config_value(t3f_user_data, "Game Data", "High Score Level");
    if(val2)
    {
      if(t3net_upload_score(app->leaderboard_submit_url, "dot_to_dot_sweep", DOT_LEADERBOARD_VERSION, "normal", "none", app->user_key, dot_leaderboard_obfuscate_score(atoi(val)), val2))
      {
        al_remove_config_key(t3f_user_data, "Game Data", "Score Uploaded");
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

  for(i = 0; i < T3F_MAX_TOUCHES; i++)
  {
    if(t3f_touch[i].pressed)
    {
      m = true;
      t3f_touch[i].pressed = false;
    }
  }
  if(app->controller.button)
  {
    m = true;
    app->controller.button = false;
  }
  if(t3f_key[ALLEGRO_KEY_ESCAPE])
  {
    m = true;
    t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
  }
  if(t3f_key[ALLEGRO_KEY_BACK])
  {
    m = true;
    t3f_key[ALLEGRO_KEY_BACK] = 0;
  }
  dot_bg_objects_logic(data, DOT_GAME_LEVEL_BASE_SPEED);
  if(m && app->current_menu == DOT_MENU_LEADERBOARD_2)
  {
    dot_menu_proc_leaderboard_back(data, 0, NULL);
    app->current_menu = DOT_MENU_LEADERBOARD_2;
  }
  else if(m)
  {
    dot_menu_proc_leaderboard_back(data, 0, NULL);
  }
  app->tick++;
  if(!app->desktop_mode)
  {
    t3f_process_gui(app->menu[app->current_menu], app);
  }
}

void dot_leaderboard_render(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    int i;
	char buf[256] = {0};
    ALLEGRO_COLOR text_color;

    al_clear_to_color(app->level_color[0]);
    al_hold_bitmap_drawing(true);
    dot_bg_objects_render(data);
    al_draw_bitmap(app->bitmap[DOT_BITMAP_BG], 0, 0, 0);
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
    if(!app->desktop_mode)
    {
        t3f_render_gui(app->menu[app->current_menu]);
    }
    al_hold_bitmap_drawing(false);
	dot_intro_render_split(data);
}
