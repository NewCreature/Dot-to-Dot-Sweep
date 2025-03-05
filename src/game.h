#ifndef DOT_GAME_H
#define DOT_GAME_H

typedef struct
{

	float x, y, z, a, s;
	float vx, vy;
	float r;
	int type;
	int timer;
	int target_tick;
	bool obscured;
	bool active;

} DOT_BALL;

typedef struct
{

	DOT_BALL ball;
	bool want_shield;
	float target_angle, old_target_angle;
	float old_mouse_x, old_mouse_y;

} DOT_PLAYER;

typedef struct
{

	float x, y, r;
	bool active;

} DOT_SHIELD;

typedef struct
{

	int x, y;

} DOT_PARTICLE_LIST_ITEM;

#define DOT_MAX_PARTICLE_LIST_ITEMS 384

/* store a list of particle positions to generate particles from */
typedef struct
{

	DOT_PARTICLE_LIST_ITEM item[DOT_MAX_PARTICLE_LIST_ITEMS];
	int items;

} DOT_PARTICLE_LIST;

#define DOT_GAME_MAX_LEVELS            10
#define DOT_GAME_COMBO_TIME           120
#define DOT_GAME_BASE_POINTS         1000
#define DOT_GAME_COMBO_POINTS         250
#define DOT_GAME_TARGET_TICKS          30
#define DOT_GAME_EXTRA_LIFE_POINTS  75000
#define DOT_GAME_MAX_MODES              2

#define DOT_GAME_MAX_BALLS      256
#define DOT_GAME_STATE_START      0
#define DOT_GAME_STATE_PLAY       1
#define DOT_GAME_STATE_PAUSE_MENU 3
#define DOT_GAME_STATE_DONE       4

#define DOT_GAME_EMO_STATE_NORMAL 0
#define DOT_GAME_EMO_STATE_BLINK  1
#define DOT_GAME_EMO_STATE_WOAH   2
#define DOT_GAME_EMO_STATE_DEAD   3

#define DOT_GAME_PLAYFIELD_WIDTH  540
#define DOT_GAME_PLAYFIELD_HEIGHT 440

#define DOT_GAME_LEVEL_BASE_BALLS       16
#define DOT_GAME_LEVEL_BALLS_INC         4
#define DOT_GAME_LEVEL_BASE_BLACK_BALLS  2
#define DOT_GAME_LEVEL_BLACK_BALLS_INC   1
#define DOT_GAME_LEVEL_BASE_SPEED      1.0
#define DOT_GAME_LEVEL_TOP_SPEED       2.0

#define DOT_GAME_SHIELD_MAX_SIZE        96
#define DOT_GAME_BALL_SIZE            16.0

#define DOT_GAME_TOUCH_START_X (DOT_GAME_SHIELD_MAX_SIZE + DOT_GAME_BALL_SIZE * 2)
#define DOT_GAME_TOUCH_START_Y (DOT_GAME_SHIELD_MAX_SIZE + DOT_GAME_BALL_SIZE * 2)
#define DOT_GAME_TOUCH_END_X (app->menu_view->virtual_width - DOT_GAME_SHIELD_MAX_SIZE - DOT_GAME_BALL_SIZE * 2)
#define DOT_GAME_TOUCH_END_Y (app->menu_view->virtual_height - DOT_GAME_SHIELD_MAX_SIZE - DOT_GAME_BALL_SIZE * 2)

typedef struct
{

	int state;
	int state_tick;
	int pause_state;
	float speed, speed_inc;
	ALLEGRO_COLOR bg_color;
	ALLEGRO_COLOR old_bg_color;
	float bg_color_fade;
	int tick;
	int extra_life_tick;

	/* player data */
	int mode;
	int lives_up_threshold;
	int lives;
	int level;
	int ascore, score, high_score[DOT_GAME_MAX_MODES];
	int combo;
	bool level_start;
	int emo_state;
	int emo_tick;
	bool a_combo_broken;
	int a_bob_and_weave_ticks;
	int a_start_lives;
	int a_oops_ticks;
	int a_colored_balls_remaining;

	DOT_BALL ball[DOT_GAME_MAX_BALLS];
	int ball_count;
	DOT_PLAYER player;
	DOT_SHIELD shield;

	/* cheats */
	bool cheats_enabled;
	int start_level;
	float speed_multiplier;
	int start_lives;

} DOT_GAME;

void dot_game_initialize(void * data, bool demo_seed, int mode);
void dot_game_exit(void * data, bool from_menu);
void dot_game_emo_logic(void * data);
void dot_game_logic(void * data);
void dot_game_render_hud(void * data);
void dot_game_render(void * data);

#endif
