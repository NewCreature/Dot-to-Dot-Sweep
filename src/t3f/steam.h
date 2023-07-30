#ifndef T3F_STEAM_H
#define T3F_STEAM_H

#ifdef __cplusplus
   extern "C" {
#endif

bool t3f_init_steam_integration(void);
void t3f_shutdown_steam_integration(void);
bool t3f_show_steam_text_input(int x, int y, int width, int height);

#ifdef __cplusplus
   }
#endif

#endif