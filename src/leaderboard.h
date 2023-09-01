#ifndef DOT_LEADERBOARD_H
#define DOT_LEADERBOARD_H

unsigned long dot_leaderboard_obfuscate_score(unsigned long score);
unsigned long dot_leaderboard_unobfuscate_score(unsigned long score);
bool dot_get_leaderboard_user_key(void * data);
void dot_upload_current_high_score(void * data);
int dot_get_leaderboard_spot(T3NET_LEADERBOARD * lp, const char * name, unsigned long score);
void dot_leaderboard_logic(void * data);
void dot_leaderboard_render(void * data);

#endif
