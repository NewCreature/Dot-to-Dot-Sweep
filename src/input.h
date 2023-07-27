#ifndef D2D_INPUT_H
#define D2D_INPUT_H

#include "t3f/input.h"

typedef struct
{

  T3F_INPUT_HANDLER * input_handler;

  /* auto select joystick and stick based on joystick events */
  int current_joy;

  float axis_x;
  float axis_y;
  bool axis_x_pressed;
  bool axis_y_pressed;
  bool button;
  float dead_zone;
} DOT_INPUT_DATA;

bool dot_initialize_input(DOT_INPUT_DATA * ip);
void dot_destroy_input(DOT_INPUT_DATA * ip);
void dot_handle_joystick_event(DOT_INPUT_DATA * ip, ALLEGRO_EVENT * ep);
void dot_read_input(DOT_INPUT_DATA * ip);
void dot_clear_input(DOT_INPUT_DATA * ip);

#endif