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

static void remap_controller(T3F_INPUT_HANDLER * input_handler, int device_index, float dead_zone)
{
  t3f_map_input_for_xbox_controller(input_handler, device_index);
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].binding[0].dead_zone = dead_zone;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].binding[0].dead_zone = dead_zone;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].binding[0].dead_zone = dead_zone;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].binding[0].dead_zone = dead_zone;
}

void dot_handle_joystick_event(DOT_INPUT_DATA * ip, ALLEGRO_EVENT * ep)
{
  int new_joy;
  int min_joystick = 0;

  #ifdef ALLEGRO_ANDROID
    min_joystick = 1;
  #endif

  if(ep->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
  {
    new_joy = t3f_get_joystick_number(ep->joystick.id);
    if(new_joy >= min_joystick && new_joy != ip->current_joy && fabs(ep->joystick.pos) >= ip->dead_zone)
    {
      remap_controller(ip->input_handler, new_joy, ip->dead_zone);
      ip->current_joy = new_joy;
      ip->current_joy_handle = t3f_joystick[new_joy];
    }
  }
  else if(ep->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN)
  {
    new_joy = t3f_get_joystick_number(ep->joystick.id);
    if(new_joy >= min_joystick && new_joy != ip->current_joy)
    {
      remap_controller(ip->input_handler, new_joy, ip->dead_zone);
      ip->current_joy = new_joy;
      ip->current_joy_handle = t3f_joystick[new_joy];
    }
  }
  else if(ep->type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION)
  {
    int i;

    if(al_get_num_joysticks() > 0 || ip->current_joy_handle)
    {
      for(i = 0; i < al_get_num_joysticks(); i++)
      {
        if(ip->current_joy_handle == al_get_joystick(i))
        {
          break;
        }
      }
      if(i == al_get_num_joysticks())
      {
        ip->current_joy = -1;
        ip->current_joy_handle = NULL;
        ip->current_joy_disconnected = true;
      }
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
  if(t3f_key_held(ALLEGRO_KEY_UP) || t3f_key_held(ALLEGRO_KEY_W))
  {
    ip->axis_y = -1.0;
  }
  if(t3f_key_pressed(ALLEGRO_KEY_UP) || t3f_key_pressed(ALLEGRO_KEY_W))
  {
    ip->axis_y = -1.0;
    ip->axis_y_pressed = true;
    t3f_use_key_press(ALLEGRO_KEY_UP);
    t3f_use_key_press(ALLEGRO_KEY_W);
  }
  if(t3f_key_held(ALLEGRO_KEY_DOWN) || t3f_key_held(ALLEGRO_KEY_S))
  {
    ip->axis_y = 1.0;
  }
  if(t3f_key_pressed(ALLEGRO_KEY_DOWN) || t3f_key_pressed(ALLEGRO_KEY_S))
  {
    ip->axis_y = 1.0;
    ip->axis_y_pressed = true;
    t3f_use_key_press(ALLEGRO_KEY_DOWN);
    t3f_use_key_press(ALLEGRO_KEY_S);
  }
  if(t3f_key_held(ALLEGRO_KEY_LEFT) || t3f_key_held(ALLEGRO_KEY_A))
  {
    ip->axis_x = -1.0;
  }
  if(t3f_key_pressed(ALLEGRO_KEY_LEFT) || t3f_key_pressed(ALLEGRO_KEY_A))
  {
    ip->axis_x = -1.0;
    ip->axis_x_pressed = true;
    t3f_use_key_press(ALLEGRO_KEY_LEFT);
    t3f_use_key_press(ALLEGRO_KEY_A);
  }
  if(t3f_key_held(ALLEGRO_KEY_RIGHT) || t3f_key_held(ALLEGRO_KEY_D))
  {
    ip->axis_x = 1.0;
  }
  if(t3f_key_pressed(ALLEGRO_KEY_RIGHT) || t3f_key_pressed(ALLEGRO_KEY_D))
  {
    ip->axis_x = 1.0;
    ip->axis_x_pressed = true;
    t3f_use_key_press(ALLEGRO_KEY_RIGHT);
    t3f_use_key_press(ALLEGRO_KEY_D);
  }
  for(i = 1; i < ALLEGRO_KEY_MAX; i++)
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
      /* work around MacOS shift key issue */
      #ifdef ALLEGRO_MACOSX
        case ALLEGRO_KEY_LSHIFT:
        case ALLEGRO_KEY_RSHIFT:
        {
          if(t3f_key_pressed(i))
          {
            ip->button = true;
            t3f_use_key_press(i);
          }
          break;
        }
      #endif
      default:
      {
        if(t3f_key_pressed(i))
        {
          ip->button = true;
          t3f_use_key_press(i);
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
    for(j = T3F_GAMEPAD_LEFT_TRIGGER; j < ip->input_handler->elements; j++)
    {
      if(ip->input_handler->element[j].pressed)
      {
        ip->button = true;
        break;
      }
    }

    if(fabs(ip->input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].val) > 0.0)
    {
      ip->axis_x = ip->input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].val;
      if(ip->input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].pressed)
      {
        ip->axis_x_pressed = true;
      }
    }
    if(fabs(ip->input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].val) > 0.0)
    {
      ip->axis_x = ip->input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].val;
      if(ip->input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].pressed)
      {
        ip->axis_x_pressed = true;
      }
    }
    if(ip->input_handler->element[T3F_GAMEPAD_DPAD_LEFT].held)
    {
      ip->axis_x = -1.0;
      if(ip->input_handler->element[T3F_GAMEPAD_DPAD_LEFT].pressed)
      {
        ip->axis_x_pressed = true;
      }
    }
    if(ip->input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].held)
    {
      ip->axis_x = 1.0;
      if(ip->input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].pressed)
      {
        ip->axis_x_pressed = true;
      }
    }
    if(fabs(ip->input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].val) > 0.0)
    {
      ip->axis_y += ip->input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].val;
      if(ip->input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].pressed)
      {
        ip->axis_y_pressed = true;
      }
    }
    if(fabs(ip->input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].val) > 0.0)
    {
      ip->axis_y += ip->input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].val;
      if(ip->input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].pressed)
      {
        ip->axis_y_pressed = true;
      }
    }
    if(ip->input_handler->element[T3F_GAMEPAD_DPAD_UP].held)
    {
      ip->axis_y = -1.0;
      if(ip->input_handler->element[T3F_GAMEPAD_DPAD_UP].pressed)
      {
        ip->axis_y_pressed = true;
      }
    }
    if(ip->input_handler->element[T3F_GAMEPAD_DPAD_DOWN].held)
    {
      ip->axis_y = 1.0;
      if(ip->input_handler->element[T3F_GAMEPAD_DPAD_DOWN].pressed)
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
  T3F_INPUT_HANDLER * old_input_handler = ip->input_handler;
  memset(ip, 0, sizeof(DOT_INPUT_DATA));
  ip->input_handler = old_input_handler;
}
