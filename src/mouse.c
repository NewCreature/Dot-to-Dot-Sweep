#include "t3f/t3f.h"

static bool _dot_mouse_enabled = true;
static bool _dot_mouse_showing = true;
static bool _dot_mouse_soft = false;

void dot_enable_soft_cursor(bool onoff)
{
  if(onoff)
  {
    _dot_mouse_soft = true;
    if(_dot_mouse_showing)
    {
      al_hide_mouse_cursor(t3f_display);
    }
  }
  else
  {
    _dot_mouse_soft = false;
    if(_dot_mouse_showing)
    {
      al_show_mouse_cursor(t3f_display);
    }
  }
}

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
      if(!_dot_mouse_soft)
      {
        al_show_mouse_cursor(t3f_display);
      }
      _dot_mouse_showing = true;
    }
  }
  else
  {
    if(_dot_mouse_showing)
    {
      if(!_dot_mouse_soft)
      {
        al_hide_mouse_cursor(t3f_display);
      }
      _dot_mouse_showing = false;
    }
  }
}

void dot_render_cursor(T3F_BITMAP * bp, float ox, float oy)
{
  if(_dot_mouse_soft && _dot_mouse_enabled)
  {
    t3f_draw_scaled_bitmap(bp, t3f_color_white, t3f_get_mouse_x() + ox, t3f_get_mouse_y() + oy, 0.0, 24.0, 24.0, 0);
  }
}
