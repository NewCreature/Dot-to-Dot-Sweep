#ifndef D2D_INSTANCE_H
#define D2D_INSTANCE_H

#include "t3f/t3f.h"
#include "t3f/rng.h"
#include "t3f/gui.h"
#include "t3f/achievements.h"
#include "t3net/leaderboard.h"
#include "defines.h"
#include "game.h"
#include "particle.h"
#include "bg_object.h"
#include "credits.h"
#include "input.h"

/* structure to hold all of our app-specific data */
typedef struct
{

    /* data */
    T3F_VIEW * main_view;
    T3F_VIEW * menu_view;
    T3F_BITMAP * bitmap[DOT_MAX_BITMAPS];
    ALLEGRO_SAMPLE * sample[DOT_MAX_SAMPLES];
    T3F_FONT * font[DOT_MAX_FONTS];
    ALLEGRO_COLOR dot_color[16];
    ALLEGRO_COLOR level_color[10];
    T3F_ATLAS * atlas;
    bool music_enabled;
    int graphics_size_multiplier;
    T3F_ACHIEVEMENTS_LIST * achievements;
    bool steam_running;
    bool on_steam_deck;
    bool reset_steam_stats;
    int game_mode;

    /* program state */
    int state;
    int tick;
    T3F_RNG_STATE rng_state;
    bool desktop_mode;
    bool entering_name;
    bool first_run;

    /* input state */
    int input_type;
    bool want_controller;
    bool want_mouse;
    bool want_touch;
    int touch_id;
    float touch_x, touch_y;
    float old_touch_x, old_touch_y;
    bool start_touch;
    float mouse_sensitivity;
    DOT_INPUT_DATA controller;
    int mickey_ticks;
    int touch_cooldown_ticks;

    /* game state */
    DOT_GAME game;

    /* particle effects */
    DOT_PARTICLE particle[DOT_MAX_PARTICLES];
    int current_particle;
    DOT_PARTICLE * point_particle[DOT_MAX_PARTICLES];
    int point_particles;
    DOT_PARTICLE * splat_particle[DOT_MAX_PARTICLES];
    int splat_particles;
    DOT_BG_OBJECT bg_object[DOT_MAX_BG_OBJECTS];
    DOT_PARTICLE_LIST number_particle_list[10];
    DOT_PARTICLE_LIST extra_life_particle_list;

    /* leaderboard stuff */
    char leaderboard_get_user_key_url[1024];
    char leaderboard_set_user_name_url[1024];
    char leaderboard_submit_url[1024];
    char leaderboard_retrieve_url[1024];
    char user_key[128];
    char user_name[256];
    bool tried_user_name_upload;
    bool upload_scores;
    T3NET_LEADERBOARD * leaderboard;
    int leaderboard_tick;
    int leaderboard_spot;

    /* menu data */
    T3F_GUI * menu[DOT_MAX_MENUS];
    char mouse_menu_sensitivity_text[32];
    int current_menu;
    int intro_state;
    float logo_ox;
    float credits_ox;
    bool menu_showing;
    int previous_element;

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
