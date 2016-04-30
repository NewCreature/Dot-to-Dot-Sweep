#ifndef D2D_DEFINES_H
#define D2D_DEFINES_H

#define DOT_MAX_BITMAPS          256
#define DOT_BITMAP_BALL_RED        0
#define DOT_BITMAP_BALL_GREEN      1
#define DOT_BITMAP_BALL_BLUE       2
#define DOT_BITMAP_BALL_PURPLE     3
#define DOT_BITMAP_BALL_YELLOW     4
#define DOT_BITMAP_BALL_ORANGE     5
#define DOT_BITMAP_BALL_BLACK      6
#define DOT_BITMAP_BALL_EYES       8
#define DOT_BITMAP_COMBO           9
#define DOT_BITMAP_LOGO           10
#define DOT_BITMAP_ATLAS_BOUNDARY DOT_BITMAP_LOGO

#define DOT_MAX_SAMPLES      256
#define DOT_SAMPLE_START       0
#define DOT_SAMPLE_GRAB        1
#define DOT_SAMPLE_LOSE        2
#define DOT_SAMPLE_GO          3

#define DOT_MAX_FONTS          4
#define DOT_FONT_16            0
#define DOT_FONT_32            1

#define DOT_MAX_MENUS          4
#define DOT_MENU_MAIN          0
#define DOT_MENU_LEADERBOARD   1
#define DOT_MENU_PROFILE       2

#define DOT_MENU_COLOR_ENABLED  t3f_color_white
#define DOT_MENU_COLOR_DISABLED al_map_rgba_f(0.75, 0.75, 0.75, 1.0)

#define DOT_STATE_INTRO        0
#define DOT_STATE_GAME         1
#define DOT_STATE_LEADERBOARD  2

#define DOT_GAME_PLAYFIELD_WIDTH  540
#define DOT_GAME_PLAYFIELD_HEIGHT 440

#define DOT_LEADERBOARD_SUBMIT_URL   "http://www.t3-i.com/t3net2/leaderboards/insert.php"
#define DOT_LEADERBOARD_RETRIEVE_URL "http://www.t3-i.com/t3net2/leaderboards/query.php"
#define DOT_LEADERBOARD_VERSION      "1.0"

#define DOT_MUSIC_TITLE "data/music/title.xm"
#define DOT_MUSIC_BGM   "data/music/bgm.xm"

#endif
