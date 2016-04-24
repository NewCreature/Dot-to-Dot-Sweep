#include "t3f/t3f.h"

void dot_shadow_text(ALLEGRO_FONT * font, ALLEGRO_COLOR color, ALLEGRO_COLOR shadow, float x, float y, float sx, float sy, int flags, const char * text)
{
	al_draw_text(font, shadow, x + sx, y + sy, flags, text);
	al_draw_text(font, color, x, y, flags, text);
}
