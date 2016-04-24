#ifndef D2D_INSTANCE_H
#define D2D_INSTANCE_H

#include "t3f/t3f.h"
#include "t3f/rng.h"
#include "t3f/gui.h"
#include "defines.h"
#include "game.h"

/* structure to hold all of our app-specific data */
typedef struct
{

    /* data */
    ALLEGRO_BITMAP * bitmap[DOT_MAX_BITMAPS];
    ALLEGRO_SAMPLE * sample[DOT_MAX_SAMPLES];
    ALLEGRO_FONT * font[DOT_MAX_FONTS];
    T3F_ATLAS * atlas;
    T3F_GUI * menu;

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

} APP_INSTANCE;

#endif
