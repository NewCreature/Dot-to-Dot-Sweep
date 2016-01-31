#include "t3f/t3f.h"
#include "instance.h"
#include "game.h"

void dot_shadow_text(ALLEGRO_FONT * font, ALLEGRO_COLOR color, ALLEGRO_COLOR shadow, float x, float y, float sx, float sy, int flags, const char * text);

void dot_intro_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->tick++;
	if(app->touch_id >= 0)
	{
		srand(time(0));
		dot_game_initialize(data);
		t3f_touch[app->touch_id].active = false;
	}
	else if(t3f_key[ALLEGRO_KEY_ESCAPE] || t3f_key[ALLEGRO_KEY_BACK])
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
	for(i = 0; i < 480 / 16; i++)
	{
		al_draw_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + i % 6], i * 16, sin((float)(i * 2 + app->tick) / 10.0) * 32 + 64, 0);
		al_draw_bitmap(app->bitmap[DOT_BITMAP_BALL_RED + i % 6], i * 16, cos((float)(i * 2 + app->tick) / 10.0) * 32 + 480 - 64 - 16 - 1, 0);
	}
	dot_game_render_hud(data);
	dot_shadow_text(app->font, t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 240, 240 - 8, 2, 2, ALLEGRO_ALIGN_CENTRE, "Click anywhere to begin...");
	al_hold_bitmap_drawing(false);
}
