#include "t3f/t3f.h"

static bool _dot_mouse_enabled = true;
static bool _dot_mouse_showing = true;

void dot_enable_mouse_cursor(bool onoff)
{
  _dot_mouse_enabled = onoff;
}

void dot_mouse_cursor_logic(void)
{
  if(_dot_mouse_enabled)
  {
    if(!_dot_mouse_showing)
    {
      al_show_mouse_cursor(t3f_display);
      _dot_mouse_showing = true;
    }
  }
  else
  {
    if(_dot_mouse_showing)
    {
      al_hide_mouse_cursor(t3f_display);
      _dot_mouse_showing = false;
    }
  }
}
