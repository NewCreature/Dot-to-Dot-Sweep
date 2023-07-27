#include "t3f.h"
#include "input.h"

static bool _t3f_input_initialized = false;

bool t3f_initialize_input(int flags)
{
  if(!_t3f_input_initialized)
  {
    _t3f_input_initialized = true;
  }
  return true;
}

void t3f_deinitialize_input(void)
{
  if(_t3f_input_initialized)
  {
    _t3f_input_initialized = false;
  }
}

T3F_INPUT_HANDLER * t3f_create_input_handler(int type)
{
  T3F_INPUT_HANDLER * input_handler = NULL;

  input_handler = malloc(sizeof(T3F_INPUT_HANDLER));
  if(!input_handler)
  {
    goto fail;
  }
  memset(input_handler, 0, sizeof(T3F_INPUT_HANDLER));

  input_handler->type = type;
  switch(input_handler->type)
  {
    case T3F_INPUT_HANDLER_TYPE_GAMEPAD:
    {
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Analog X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Analog Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Analog X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Analog Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Trigger
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Trigger
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // A
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // B
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // L
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // R
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // L3
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // R3
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Select
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Start
      break;
    }
  }
  return input_handler;

  fail:
  {
    if(input_handler)
    {
      free(input_handler);
    }
    return NULL;
  }
}

void t3f_destroy_input_handler(T3F_INPUT_HANDLER * input_handler)
{
  free(input_handler);
}

bool t3f_add_input_handler_element(T3F_INPUT_HANDLER * input_handler, int type)
{
  T3F_INPUT_HANDLER_ELEMENT * element = NULL;
  int i;

  element = malloc(sizeof(T3F_INPUT_HANDLER_ELEMENT) * (input_handler->elements + 1));
  if(!element)
  {
    goto fail;
  }
  for(i = 0; i < input_handler->elements; i++)
  {
    memcpy(&element[i], &input_handler->element[i], sizeof(T3F_INPUT_HANDLER_ELEMENT));
  }
  memset(&element[i], 0, sizeof(T3F_INPUT_HANDLER_ELEMENT));
  element[i].type = type;
  free(input_handler->element);
  input_handler->element = element;
  input_handler->elements++;

  return true;

  fail:
  {
    if(element)
    {
      free(element);
    }
    return false;
  }
}

void t3f_bind_input_handler_element(T3F_INPUT_HANDLER * input_handler, int element, int device_type, int device_number, int device_element)
{
  input_handler->element[element].device_type = device_type;
  input_handler->element[element].device_number = device_number;
  input_handler->element[element].device_element = device_element;
}

bool t3f_map_input_for_xbox_controller(T3F_INPUT_HANDLER * input_handler, int joystick)
{
  if(input_handler->type != T3F_INPUT_HANDLER_TYPE_GAMEPAD)
  {
    return false;
  }

  #ifdef ALLEGRO_MACOSX

    /* Left Analog X */
    input_handler->element[0].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[0].device_number = joystick;
    input_handler->element[0].device_element = 0;
    input_handler->element[0].dead_zone = 0.15;
    input_handler->element[0].threshold = 0.5;

    /* Left Analog Y */
    input_handler->element[1].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[1].device_number = joystick;
    input_handler->element[1].device_element = 1;
    input_handler->element[1].dead_zone = 0.15;
    input_handler->element[1].threshold = 0.5;

    /* Right Analog X */
    input_handler->element[2].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[2].device_number = joystick;
    input_handler->element[2].device_element = 2;
    input_handler->element[2].dead_zone = 0.15;
    input_handler->element[2].threshold = 0.5;

    /* Right Analog Y */
    input_handler->element[3].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[3].device_number = joystick;
    input_handler->element[3].device_element = 3;
    input_handler->element[3].dead_zone = 0.15;
    input_handler->element[3].threshold = 0.5;

    /* D-Pad X */
    input_handler->element[4].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[4].device_number = joystick;
    input_handler->element[4].device_element = 6;
    input_handler->element[4].dead_zone = 0.0;
    input_handler->element[4].threshold = 0.1;

    /* D-Pad Y */
    input_handler->element[5].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[5].device_number = joystick;
    input_handler->element[5].device_element = 7;
    input_handler->element[5].dead_zone = 0.0;
    input_handler->element[5].threshold = 0.1;

    /* Left Trigger */
    input_handler->element[6].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[6].device_number = joystick;
    input_handler->element[6].device_element = 4;
    input_handler->element[6].dead_zone = 0.0;
    input_handler->element[6].threshold = 0.5;

    /* Right Trigger */
    input_handler->element[7].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[7].device_number = joystick;
    input_handler->element[7].device_element = 5;
    input_handler->element[7].dead_zone = 0.0;
    input_handler->element[7].threshold = 0.5;

    /* A */
    input_handler->element[8].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[8].device_number = joystick;
    input_handler->element[8].device_element = 8;

    /* B */
    input_handler->element[9].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[9].device_number = joystick;
    input_handler->element[9].device_element = 9;

    /* X */
    input_handler->element[10].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[10].device_number = joystick;
    input_handler->element[10].device_element = 10;

    /* Y */
    input_handler->element[11].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[11].device_number = joystick;
    input_handler->element[11].device_element = 11;

    /* L */
    input_handler->element[12].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[12].device_number = joystick;
    input_handler->element[12].device_element = 12;

    /* R */
    input_handler->element[13].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[13].device_number = joystick;
    input_handler->element[13].device_element = 13;

    /* L3 */
    input_handler->element[14].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[14].device_number = joystick;
    input_handler->element[14].device_element = 16;

    /* R3 */
    input_handler->element[15].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[15].device_number = joystick;
    input_handler->element[15].device_element = 17;

    /* Start */
    input_handler->element[16].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[16].device_number = joystick;
    input_handler->element[16].device_element = 14;

    /* Select */
    input_handler->element[17].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
    input_handler->element[17].device_number = joystick;
    input_handler->element[17].device_element = 15;

  #else
  #endif

  return true;
}

static void update_input_device(int device)
{
  #ifdef ALLEGRO_MACOSX
    if(t3f_joystick_state_updated[device])
    {
      /* up */
      if(t3f_joystick_state[device].stick[4].axis[0] >= 1.0 && t3f_joystick_state[device].stick[4].axis[1] <= -1.0)
      {
        t3f_joystick_state[device].stick[4].axis[0] = 0.0;
        t3f_joystick_state[device].stick[4].axis[1] = -1.0;
      }

      /* down */
      else if(t3f_joystick_state[device].stick[4].axis[0] <= -1.0 && t3f_joystick_state[device].stick[4].axis[1] >= 1.0)
      {
        t3f_joystick_state[device].stick[4].axis[0] = 0.0;
        t3f_joystick_state[device].stick[4].axis[1] = 1.0;
      }

      /* left */
      else if(t3f_joystick_state[device].stick[4].axis[0] <= -1.0 && t3f_joystick_state[device].stick[4].axis[1] <= -1.0)
      {
        t3f_joystick_state[device].stick[4].axis[0] = -1.0;
        t3f_joystick_state[device].stick[4].axis[1] = 0.0;
      }

      /* right */
      else if(t3f_joystick_state[device].stick[4].axis[0] >= 1.0 && t3f_joystick_state[device].stick[4].axis[1] >= 1.0)
      {
        t3f_joystick_state[device].stick[4].axis[0] = 1.0;
        t3f_joystick_state[device].stick[4].axis[1] = 0.0;
      }

      /* up-right */
      else if(t3f_joystick_state[device].stick[4].axis[0] >= 1.0)
      {
        t3f_joystick_state[device].stick[4].axis[0] = 1.0;
        t3f_joystick_state[device].stick[4].axis[1] = -1.0;
      }

      /* down-right */
      else if(t3f_joystick_state[device].stick[4].axis[1] >= 1.0)
      {
        t3f_joystick_state[device].stick[4].axis[0] = 1.0;
        t3f_joystick_state[device].stick[4].axis[1] = 1.0;
      }

      /* down-left */
      else if(t3f_joystick_state[device].stick[4].axis[0] <= -1.0)
      {
        t3f_joystick_state[device].stick[4].axis[0] = -1.0;
        t3f_joystick_state[device].stick[4].axis[1] = 1.0;
      }

      /* up-left */
      else if(t3f_joystick_state[device].stick[4].axis[0] == 0.0 && t3f_joystick_state[device].stick[4].axis[1] == 0.0)
      {
        t3f_joystick_state[device].stick[4].axis[0] = -1.0;
        t3f_joystick_state[device].stick[4].axis[1] = -1.0;
      }

      /* idle */
      else
      {
        t3f_joystick_state[device].stick[4].axis[0] = 0.0;
        t3f_joystick_state[device].stick[4].axis[1] = 0.0;
      }

      /* triggers */
      t3f_joystick_state[device].stick[2].axis[0] = (t3f_joystick_state[device].stick[2].axis[0] + 1.0) / 2.0;
      t3f_joystick_state[device].stick[3].axis[0] = (t3f_joystick_state[device].stick[3].axis[0] + 1.0) / 2.0;

      t3f_joystick_state_updated[device] = false;
    }
  #endif
}

static void update_input_handler_element_state_keyboard(T3F_INPUT_HANDLER_ELEMENT * element)
{
  if(t3f_key[element->device_element])
  {
    element->held = true;
    element->released = false;
    if(!element->pressed)
    {
      element->pressed = true;
    }
    else
    {
      element->pressed = false;
    }
  }
  else
  {
    element->held = false;
    element->pressed = false;
    if(!element->released)
    {
      element->released = true;
    }
    else
    {
      element->released = false;
    }
  }
}

static void update_input_handler_element_state_mouse(T3F_INPUT_HANDLER_ELEMENT * element)
{
  int button;

  switch(element->device_element)
  {
    case 0:
    {
      element->val = t3f_mouse_x;
      break;
    }
    case 1:
    {
      element->val = t3f_mouse_y;
      break;
    }
    case 2:
    {
      element->val = t3f_mouse_z;
      break;
    }
    case 3: // placeholder for mouse_w
    {
      element->val = 0;
    }
    default:
    {
      button = element->device_element - 4;
      if(button >= 0)
      {
        if(t3f_mouse_button[button])
        {
          element->held = true;
          element->released = false;
          if(!element->pressed)
          {
            element->pressed = true;
          }
          else
          {
            element->pressed = false;
          }
        }
        else
        {
          element->held = false;
          element->pressed = false;
          if(!element->released)
          {
            element->released = true;
          }
          else
          {
            element->released = false;
          }
        }
      }
      break;
    }
  }
}

static void update_input_handler_element_state_touch(T3F_INPUT_HANDLER_ELEMENT * element)
{
}

static void update_input_handler_element_joystick_cache(T3F_INPUT_HANDLER_ELEMENT * element)
{
  int i, j;

  if(element->stick_elements <= 0)
  {
    element->stick_elements = 0;
    for(i = 0; i < al_get_joystick_num_sticks(t3f_joystick[element->device_number]); i++)
    {
      for(j = 0; j < al_get_joystick_num_axes(t3f_joystick[element->device_number], i); j++)
      {
        element->stick[element->stick_elements] = i;
        element->axis[element->stick_elements] = j;
        element->stick_elements++;
      }
    }
  }
}

static void update_input_handler_element_state_joystick(T3F_INPUT_HANDLER_ELEMENT * element)
{
  update_input_handler_element_joystick_cache(element);
  update_input_device(element->device_number);
  if(element->device_element < element->stick_elements)
  {
    if(fabs(t3f_joystick_state[element->device_number].stick[element->stick[element->device_element]].axis[element->axis[element->device_element]]) >= element->dead_zone)
    {
      element->val = t3f_joystick_state[element->device_number].stick[element->stick[element->device_element]].axis[element->axis[element->device_element]];
      if(element->val < 0.0)
      {
        element->val = (element->val + element->dead_zone) / (1.0 - element->dead_zone);
      }
      else
      {
        element->val = (element->val - element->dead_zone) / (1.0 - element->dead_zone);
      }
    }
    else
    {
      element->val = 0.0;
    }
    if(fabs(element->val) >= element->threshold)
    {
      element->held = true;
      element->released = false;
      if(!element->pressed)
      {
        element->pressed = true;
      }
      else
      {
        element->pressed = false;
      }
    }
    else
    {
      element->held = false;
      element->pressed = false;
      if(!element->released)
      {
        element->released = true;
      }
      else
      {
        element->released = false;
      }
    }
  }
  else
  {
    if(t3f_joystick_state[element->device_number].button[element->device_element - element->stick_elements])
    {
      element->held = true;
      element->released = false;
      if(!element->pressed)
      {
        element->pressed = true;
      }
      else
      {
        element->pressed = false;
      }
    }
    else
    {
      element->held = false;
      element->pressed = false;
      if(!element->released)
      {
        element->released = true;
      }
      else
      {
        element->released = false;
      }
    }
  }
}

static void update_input_handler_element_state(T3F_INPUT_HANDLER_ELEMENT * element)
{
  switch(element->device_type)
  {
    case T3F_INPUT_HANDLER_DEVICE_TYPE_KEYBOARD:
    {
      update_input_handler_element_state_keyboard(element);
      break;
    }
    case T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE:
    {
      update_input_handler_element_state_mouse(element);
      break;
    }
    case T3F_INPUT_HANDLER_DEVICE_TYPE_TOUCH:
    {
      update_input_handler_element_state_touch(element);
      break;
    }
    case T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK:
    {
      update_input_handler_element_state_joystick(element);
      break;
    }
  }
}

void t3f_update_input_handler_state(T3F_INPUT_HANDLER * input_handler)
{
  int i;

  for(i = 0; i < input_handler->elements; i++)
  {
    update_input_handler_element_state(&input_handler->element[i]);    
  }
}

