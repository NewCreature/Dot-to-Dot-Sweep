#include "t3f/t3f.h"
#include "instance.h"
#include "text.h"
#include "color.h"

void dot_leaderboard_logic(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
    {
        t3net_destroy_leaderboard(app->leaderboard);
        app->leaderboard = NULL;
        app->state = DOT_STATE_INTRO;
        t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
        t3f_key[ALLEGRO_KEY_BACK] = 0;
    }
}

void dot_leaderboard_render(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    int i;
	char buf[256] = {0};;

    al_clear_to_color(app->level_color[0]);
    al_hold_bitmap_drawing(true);
    dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 4, 4 + 0, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, "Name");
    dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 4, 4 + 32, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, "----");
    dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, 4 + 0, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_CENTRE, "Level");
    dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, 4 + 32, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_CENTRE, "-----");
    dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width - 4, 4 + 0, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_RIGHT, "Score  ");
    dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width - 4, 4 + 32, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_RIGHT, "-------");
    for(i = 0; i < app->leaderboard->entries; i++)
    {
        dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 4, 4 + (i + 3) * 32, DOT_SHADOW_OX, DOT_SHADOW_OY, 0, app->leaderboard->entry[i]->name);
        sprintf(buf, "%07lu", app->leaderboard->entry[i]->score);
        dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width - 4, 4 + (i + 3) * 32, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_RIGHT, buf);
        dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, 4 + (i + 3) * 32, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_CENTRE, app->leaderboard->entry[i]->extra);
    }
    al_hold_bitmap_drawing(false);
}
