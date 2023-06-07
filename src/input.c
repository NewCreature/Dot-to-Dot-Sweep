#include "t3f/t3f.h"
#include "input.h"

void dot_handle_joystick_event(DOT_INPUT_DATA * ip, ALLEGRO_EVENT * ep)
{
  if(ep->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
  {
    ip->current_joy = t3f_get_joystick_number(ep->joystick.id);
    if(ip->current_joy >= 0)
    {
      ip->current_stick = ep->joystick.stick;
    }
  }
  else if(ep->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN)
  {
    ip->current_joy = t3f_get_joystick_number(ep->joystick.id);
  }
}

void dot_read_input(DOT_INPUT_DATA * ip)
{
  int i, j;
  float val;
  bool using_axis = false; // flag if axis is being used
  bool using_button = false;
  double a;

  ip->axis_x = 0.0;
  ip->axis_y = 0.0;
  ip->button = false;
 
  /* read keyboard */
  if(t3f_key[ALLEGRO_KEY_UP] || t3f_key[ALLEGRO_KEY_W])
  {
    if(!(ip->axes_blocked))
    {
      ip->axis_y = -1.0;
    }
    using_axis = true;
  }
  if(t3f_key[ALLEGRO_KEY_DOWN] || t3f_key[ALLEGRO_KEY_S])
  {
    if(!(ip->axes_blocked))
    {
      ip->axis_y = 1.0;
    }
    using_axis = true;
  }
  if(t3f_key[ALLEGRO_KEY_LEFT] || t3f_key[ALLEGRO_KEY_A])
  {
    if(!(ip->axes_blocked))
    {
      ip->axis_x = -1.0;
    }
    using_axis = true;
  }
  if(t3f_key[ALLEGRO_KEY_RIGHT] || t3f_key[ALLEGRO_KEY_D])
  {
    if(!(ip->axes_blocked))
    {
      ip->axis_x = 1.0;
    }
    using_axis = true;
  }
  for(i = 0; i < ALLEGRO_KEY_MAX; i++)
  {
    switch(i)
    {
      case ALLEGRO_KEY_ESCAPE:
      case ALLEGRO_KEY_BACK:
      case ALLEGRO_KEY_UP:
      case ALLEGRO_KEY_DOWN:
      case ALLEGRO_KEY_LEFT:
      case ALLEGRO_KEY_RIGHT:
      case ALLEGRO_KEY_W:
      case ALLEGRO_KEY_D:
      case ALLEGRO_KEY_A:
      case ALLEGRO_KEY_S:
      {
        break;
      }
      default:
      {
        if(t3f_key[i])
        {
          if(!(ip->button_blocked))
          {
            ip->button = true;
          }
          using_button = true;
          i = ALLEGRO_KEY_MAX;
        }
        break;
      }
    }
  }

  /* read buttons */
  if(ip->current_joy >= 0 && t3f_joystick[ip->current_joy])
  {
    for(j = 0; j < al_get_joystick_num_buttons(t3f_joystick[ip->current_joy]); j++)
    {
      if(t3f_joystick_state[ip->current_joy].button[j])
      {
        if(!(ip->button_blocked))
        {
          ip->button = true;
        }
        using_button = true;
        break;
      }
    }

    if(al_get_joystick_num_axes(t3f_joystick[ip->current_joy], ip->current_stick) > 1)
    {
      val = t3f_joystick_state[ip->current_joy].stick[ip->current_stick].axis[0];
      if(fabs(val) > ip->dead_zone)
      {
        if(!(ip->axes_blocked))
        {
          ip->axis_x += (val - ip->dead_zone) / (1.0 - ip->dead_zone);
        }
        using_axis = true;
      }
      val = t3f_joystick_state[ip->current_joy].stick[ip->current_stick].axis[1];
      if(fabs(val) > ip->dead_zone)
      {
        if(!(ip->axes_blocked))
        {
          ip->axis_y += (val - ip->dead_zone) / (1.0 - ip->dead_zone);
        }
        using_axis = true;
      }
    }
  }
  if(!using_axis)
  {
    ip->axes_blocked = false;
  }
  if(!using_button)
  {
    ip->button_blocked = false;
  }

  /* keep axis values within range */
  if(ip->axis_x < -1.0)
  {
    ip->axis_x = -1.0;
  }
  if(ip->axis_x > 1.0)
  {
    ip->axis_x = 1.0;
  }
  if(ip->axis_y < -1.0)
  {
    ip->axis_y = -1.0;
  }
  if(ip->axis_y > 1.0)
  {
    ip->axis_y = 1.0;
  }
  if(t3f_distance(0.0, 0.0, ip->axis_x, ip->axis_y) > 1.0)
  {
    a = atan2(ip->axis_y, ip->axis_x);
    ip->axis_x = cos(a);
    ip->axis_y = sin(a);
  }
}
