#include "t3f/t3f.h"
#include "input.h"

bool dot_initialize_input(DOT_INPUT_DATA * ip)
{
  if(!t3f_initialize_input(0))
  {
    return false;
  }
  ip->input_handler = t3f_create_input_handler(T3F_INPUT_HANDLER_TYPE_GAMEPAD);
  if(!ip->input_handler)
  {
    return false;
  }
  if(al_get_num_joysticks() > 0)
  {
    t3f_map_input_for_xbox_controller(ip->input_handler, 0);
  }
  return true;
}

void dot_destroy_input(DOT_INPUT_DATA * ip)
{
  t3f_destroy_input_handler(ip->input_handler);
  t3f_deinitialize_input();
}

void dot_handle_joystick_event(DOT_INPUT_DATA * ip, ALLEGRO_EVENT * ep)
{
  int new_joy;

  if(ep->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
  {
    new_joy = t3f_get_joystick_number(ep->joystick.id);
    if(new_joy >= 0 && new_joy != ip->current_joy && fabs(ep->joystick.pos) >= ip->dead_zone)
    {
      t3f_map_input_for_xbox_controller(ip->input_handler, new_joy);
      ip->current_joy = new_joy;
    }
  }
  else if(ep->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN)
  {
    new_joy = t3f_get_joystick_number(ep->joystick.id);
    if(new_joy >= 0 && new_joy != ip->current_joy)
    {
      t3f_map_input_for_xbox_controller(ip->input_handler, new_joy);
      ip->current_joy = t3f_get_joystick_number(ep->joystick.id);
    }
  }
}

void dot_read_input(DOT_INPUT_DATA * ip)
{
  int i, j;
  float a;

  ip->axis_x = 0.0;
  ip->axis_y = 0.0;
  ip->button = false;

  /* read keyboard */
  if(t3f_key[ALLEGRO_KEY_UP] || t3f_key[ALLEGRO_KEY_W])
  {
    ip->axis_y = -1.0;
    t3f_key[ALLEGRO_KEY_UP] = 0;
    t3f_key[ALLEGRO_KEY_W] = 0;
  }
  if(t3f_key[ALLEGRO_KEY_DOWN] || t3f_key[ALLEGRO_KEY_S])
  {
    ip->axis_y = 1.0;
    t3f_key[ALLEGRO_KEY_DOWN] = 0;
    t3f_key[ALLEGRO_KEY_S] = 0;
  }
  if(t3f_key[ALLEGRO_KEY_LEFT] || t3f_key[ALLEGRO_KEY_A])
  {
    ip->axis_x = -1.0;
    t3f_key[ALLEGRO_KEY_LEFT] = 0;
    t3f_key[ALLEGRO_KEY_A] = 0;
  }
  if(t3f_key[ALLEGRO_KEY_RIGHT] || t3f_key[ALLEGRO_KEY_D])
  {
    ip->axis_x = 1.0;
    t3f_key[ALLEGRO_KEY_RIGHT] = 0;
    t3f_key[ALLEGRO_KEY_D] = 0;
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
      case ALLEGRO_KEY_PRINTSCREEN:
      case ALLEGRO_KEY_TILDE:
      case 104:
      {
        break;
      }
      default:
      {
        if(t3f_key[i])
        {
          ip->button = true;
          t3f_key[i] = 0;
          i = ALLEGRO_KEY_MAX;
        }
        break;
      }
    }
  }

	t3f_update_input_handler_state(ip->input_handler);

  /* read buttons */
  if(ip->current_joy >= 0 && t3f_joystick[ip->current_joy])
  {
    for(j = 6; j < ip->input_handler->elements; j++)
    {
      if(ip->input_handler->element[j].pressed)
      {
        ip->button = true;
        break;
      }
    }

    if(fabs(ip->input_handler->element[0].val) > 0.0)
    {
      ip->axis_x = ip->input_handler->element[0].val;
      if(ip->input_handler->element[0].pressed)
      {
        ip->axis_x_pressed = true;
      }
    }
    if(fabs(ip->input_handler->element[2].val) > 0.0)
    {
      ip->axis_x = ip->input_handler->element[2].val;
      if(ip->input_handler->element[2].pressed)
      {
        ip->axis_x_pressed = true;
      }
    }
    if(fabs(ip->input_handler->element[4].val) > 0.0)
    {
      ip->axis_x = ip->input_handler->element[4].val;
      if(ip->input_handler->element[4].pressed)
      {
        ip->axis_x_pressed = true;
      }
    }
    if(fabs(ip->input_handler->element[1].val) > 0.0)
    {
      ip->axis_y += ip->input_handler->element[1].val;
      if(ip->input_handler->element[1].pressed)
      {
        ip->axis_y_pressed = true;
      }
    }
    if(fabs(ip->input_handler->element[3].val) > 0.0)
    {
      ip->axis_y += ip->input_handler->element[3].val;
      if(ip->input_handler->element[3].pressed)
      {
        ip->axis_y_pressed = true;
      }
    }
    if(fabs(ip->input_handler->element[5].val) > 0.0)
    {
      ip->axis_y += ip->input_handler->element[5].val;
      if(ip->input_handler->element[5].pressed)
      {
        ip->axis_y_pressed = true;
      }
    }
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

void dot_clear_input(DOT_INPUT_DATA * ip)
{
  memset(ip, 0, sizeof(DOT_INPUT_DATA));
}
