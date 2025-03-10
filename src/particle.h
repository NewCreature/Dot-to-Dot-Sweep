#ifndef DOT_PARTICLE_H
#define DOT_PARTICLE_H

typedef struct
{

	int type;
	float x, y, z;
	float vx, vy, vz;
	float gz;
	float size;
	T3F_BITMAP * bp;
	ALLEGRO_COLOR color;
	int life;
	int start_life;
	bool active;

} DOT_PARTICLE;

void dot_create_particle(DOT_PARTICLE * pp, int type, float x, float y, float z, float vx, float vy, float vz, float gz, float size, int life, T3F_BITMAP * bp, ALLEGRO_COLOR color);
void dot_particle_logic(DOT_PARTICLE * pp);
void dot_particle_render(DOT_PARTICLE * pp, T3F_BITMAP * particle_bitmap);

int dot_particle_qsort_helper(const void * p1, const void * p2);

#endif
