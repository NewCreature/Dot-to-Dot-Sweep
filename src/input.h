#ifndef D2D_INPUT_H
#define D2D_INPUT_H

typedef struct
{
  /* auto select joystick and stick based on joystick events */
  int current_joy;
  int current_stick;

  float axis_x;
  float axis_y;
  bool button;
  bool axes_blocked;
  bool button_blocked;
  float dead_zone;
} DOT_INPUT_DATA;

void dot_handle_joystick_event(DOT_INPUT_DATA * ip, ALLEGRO_EVENT * ep);
void dot_read_input(DOT_INPUT_DATA * ip);

#endif