#ifndef DOT_MOUSE_H
#define DOT_MOUSE_H

void dot_enable_soft_cursor(bool onoff);
void dot_enable_mouse_cursor(bool onoff);
void dot_mouse_cursor_logic(void);
void dot_render_cursor(T3F_BITMAP * bp);

#endif
