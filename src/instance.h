#ifndef D2D_INSTANCE_H
#define D2D_INSTANCE_H

#include "t3f/t3f.h"
#include "t3f/rng.h"
#include "t3f/gui.h"
#include "t3net/leaderboard.h"
#include "defines.h"
#include "game.h"
#include "particle.h"
#include "bg_object.h"
#include "credits.h"

/* structure to hold all of our app-specific data */
typedef struct
{

    /* data */
    ALLEGRO_BITMAP * bitmap[DOT_MAX_BITMAPS];
    ALLEGRO_SAMPLE * sample[DOT_MAX_SAMPLES];
    ALLEGRO_FONT * font[DOT_MAX_FONTS];
    ALLEGRO_COLOR color[16];
    T3F_ATLAS * atlas;
    bool music_enabled;

    /* program state */
    int state;
    int tick;
    T3F_RNG_STATE rng_state;

    /* input state */
    int touch_id;
    float touch_x;
    float touch_y;

    /* game state */
    DOT_GAME game;

    /* particle effects */
    DOT_PARTICLE particle[DOT_MAX_PARTICLES];
    int current_particle;
    DOT_PARTICLE * active_particle[DOT_MAX_PARTICLES];
    int active_particles;
    DOT_BG_OBJECT bg_object[DOT_MAX_BG_OBJECTS];

    /* leaderboard stuff */
    char user_name[256];
    bool upload_scores;
    T3NET_LEADERBOARD * leaderboard;
    int leaderboard_tick;
    int leaderboard_spot;

    /* menu data */
    T3F_GUI * menu[DOT_MAX_MENUS];
    int current_menu;
    int intro_state;
    float logo_ox;
    float credits_ox;

    /* credits data */
    DOT_CREDITS credits;

    /* demo recording */
    ALLEGRO_FILE * demo_file;
    bool demo_recording;
    unsigned long demo_seed;
    bool demo_done;
    char * demo_filename;

} APP_INSTANCE;

#endif
