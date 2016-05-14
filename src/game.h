#ifndef DOT_GAME_H
#define DOT_GAME_H

typedef struct
{

	float x, y, z, a, s;
	float vx, vy;
	float r;
	int type;
	int timer;
	bool active;

} DOT_BALL;

typedef struct
{

	DOT_BALL ball;
	bool lost_touch;

} DOT_PLAYER;

#define DOT_GAME_MAX_LEVELS   10
#define DOT_GAME_COMBO_TIME  120

#define DOT_GAME_MAX_BALLS   256
#define DOT_GAME_STATE_START   0
#define DOT_GAME_STATE_PLAY    1
#define DOT_GAME_STATE_PAUSE   2
#define DOT_GAME_STATE_DONE    3

#define DOT_GAME_LEVEL_BASE_BALLS       16
#define DOT_GAME_LEVEL_BALLS_INC         4
#define DOT_GAME_LEVEL_BASE_BLACK_BALLS  2
#define DOT_GAME_LEVEL_BLACK_BALLS_INC   1
#define DOT_GAME_LEVEL_BASE_SPEED      1.0
#define DOT_GAME_LEVEL_TOP_SPEED       2.0

#define DOT_GAME_GRAB_SPOT_SIZE 24

typedef struct
{

	int state;
	int state_tick;
	float speed, speed_inc;

	/* player data */
	int lives;
	int level;
	int ascore, score, high_score;
	int combo;

	DOT_BALL ball[DOT_GAME_MAX_BALLS];
	DOT_PLAYER player;

} DOT_GAME;

void dot_game_initialize(void * data);
void dot_game_logic(void * data);
void dot_game_render_hud(void * data);
void dot_game_render(void * data);

#endif
