#include "t3f/t3f.h"

ALLEGRO_COLOR dot_darken_color(ALLEGRO_COLOR c1, float amount)
{
	float r1, g1, b1, a1;

	al_unmap_rgba_f(c1, &r1, &g1, &b1, &a1);
	return al_map_rgba_f(r1 * amount, g1 * amount, b1 * amount, a1);
}
