#ifndef D2D_DEFINES_H
#define D2D_DEFINES_H

#define DOT_DISPLAY_WIDTH  540
#define DOT_DISPLAY_HEIGHT (app->desktop_mode ? 520 : 960)

#define DOT_EXTRA_LIFE_TEXT   "1-Up"

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
#define DOT_BITMAP_PARTICLE       10
#define DOT_BITMAP_LOGO           16
#define DOT_BITMAP_EMO_BG         17
#define DOT_BITMAP_EMO_FG         18
#define DOT_BITMAP_EMO_NORMAL     19
#define DOT_BITMAP_EMO_BLINK      20
#define DOT_BITMAP_EMO_WOAH       21
#define DOT_BITMAP_EMO_DEAD       22
#define DOT_BITMAP_TARGET         23
#define DOT_BITMAP_BG             24
#define DOT_BITMAP_HAND           25
#define DOT_BITMAP_HAND_DOWN      26
#define DOT_BITMAP_ATLAS_BOUNDARY DOT_BITMAP_TARGET
#define DOT_BITMAP_SCRATCH        27

#define DOT_BITMAP_SCRATCH_WIDTH  512
#define DOT_BITMAP_SCRATCH_HEIGHT 512

#define DOT_MAX_SAMPLES      256
#define DOT_SAMPLE_START       0
#define DOT_SAMPLE_GRAB        1
#define DOT_SAMPLE_LOSE        2
#define DOT_SAMPLE_GO          3
#define DOT_SAMPLE_SCORE       4

#define DOT_MAX_FONTS          4
#define DOT_FONT_16            0
#define DOT_FONT_32            1
#define DOT_FONT_8             2
#define DOT_FONT_14            3

#define DOT_MAX_MENUS         10
#define DOT_MENU_MAIN          0
#define DOT_MENU_LEADERBOARD   1
#define DOT_MENU_LEADERBOARD_2 2
#define DOT_MENU_PRIVACY       3
#define DOT_MENU_UPLOAD_SCORES 4
#define DOT_MENU_PROFILE       5
#define DOT_MENU_MUSIC         6
#define DOT_MENU_PAUSE         7
#define DOT_MENU_GAME_MODE     8
#define DOT_MENU_MOUSE         9

#define DOT_MENU_COLOR_ENABLED  t3f_color_white
#define DOT_MENU_COLOR_DISABLED al_map_rgba_f(0.75, 0.75, 0.75, 1.0)

#define DOT_STATE_INTRO        0
#define DOT_STATE_GAME         1
#define DOT_STATE_LEADERBOARD  2
#define DOT_STATE_PRIVACY      3

#define DOT_MAX_PARTICLES        1024
#define DOT_MAX_BG_OBJECTS        160

#define DOT_SHADOW_OX -2
#define DOT_SHADOW_OY  2

#define DOT_LEADERBOARD_VERSION      "1.0"

#define DOT_MUSIC_TITLE "data/music/verge_of_happiness_remix.xm"
#define DOT_MUSIC_BGM   "data/music/going_for_it.xm"

#define DOT_ACHIEVEMENT_GETTING_INTO_IT 0
#define DOT_ACHIEVEMENT_FULL_COMBO      1
#define DOT_ACHIEVEMENT_BOB_AND_WEAVE   2
#define DOT_ACHIEVEMENT_GETTING_GOOD    3
#define DOT_ACHIEVEMENT_OOPS            4
#define DOT_ACHIEVEMENT_SEE_IT_THROUGH  5
#define DOT_ACHIEVEMENT_SO_CLOSE        6
#define DOT_ACHIEVEMENT_GOOD_GAME       7

#define DOT_LEADERBOARD_FACTOR         67

#endif
