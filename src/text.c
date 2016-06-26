#include "t3f/t3f.h"
#include "instance.h"
#include "game.h"

void dot_shadow_text(ALLEGRO_FONT * font, ALLEGRO_COLOR color, ALLEGRO_COLOR shadow, float x, float y, float sx, float sy, int flags, const char * text)
{
	al_draw_text(font, shadow, x + sx, y + sy, flags, text);
	al_draw_text(font, color, x, y, flags, text);
}

void dot_show_message(void * data, const char * message)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3f_render(false);
	al_hold_bitmap_drawing(false);
	al_draw_filled_rectangle(0.0, 0.0, t3f_virtual_display_width, t3f_virtual_display_height, al_map_rgba_f(0.0, 0.0, 0.0, 0.75));
	dot_shadow_text(app->font[DOT_FONT_16], t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), t3f_virtual_display_width / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2 - al_get_font_line_height(app->font[DOT_FONT_16]) / 2, DOT_SHADOW_OX, DOT_SHADOW_OY, ALLEGRO_ALIGN_CENTRE, message);
	al_draw_filled_rectangle(0.0, t3f_virtual_display_width, 0.0, t3f_virtual_display_height, al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
	al_flip_display();
}
