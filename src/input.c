#include "t3f/t3f.h"

void dot_read_input(float * x, float * y, bool * button, bool * block_axes, bool * block_button)
{
  int i, j;
  float val;
  bool using_axis = false; // flag if axis is being used
  bool using_button = false;

  *x = 0.0;
  *y = 0.0;
  *button = false;
 
  /* read keyboard */
  if(t3f_key[ALLEGRO_KEY_UP] || t3f_key[ALLEGRO_KEY_W])
  {
    *y = -1.0;
    t3f_key[ALLEGRO_KEY_UP] = 0;
    t3f_key[ALLEGRO_KEY_W] = 0;
  }
  else if(t3f_key[ALLEGRO_KEY_DOWN] || t3f_key[ALLEGRO_KEY_S])
  {
    *y = 1.0;
    t3f_key[ALLEGRO_KEY_DOWN] = 0;
    t3f_key[ALLEGRO_KEY_S] = 0;
  }
  else if(t3f_key[ALLEGRO_KEY_LEFT] || t3f_key[ALLEGRO_KEY_A])
  {
    *x = -1.0;
    t3f_key[ALLEGRO_KEY_LEFT] = 0;
    t3f_key[ALLEGRO_KEY_A] = 0;
  }
  else if(t3f_key[ALLEGRO_KEY_RIGHT] || t3f_key[ALLEGRO_KEY_D])
  {
    *x = 1.0;
    t3f_key[ALLEGRO_KEY_RIGHT] = 0;
    t3f_key[ALLEGRO_KEY_A] = 0;
  }
  else
  {
    for(i = 0; i < ALLEGRO_KEY_MAX; i++)
    {
      if(i != ALLEGRO_KEY_ESCAPE && i != ALLEGRO_KEY_BACK)
      {
        if(t3f_key[i])
        {
          *button = true;
          t3f_key[i] = 0;
          break;
        }
      }
    }
  }

  /* read controllers */
  for(i = 0; i < al_get_num_joysticks(); i++)
  {
    /* read buttons */
    for(j = 0; j < al_get_joystick_num_buttons(t3f_joystick[i]); j++)
    {
      if(t3f_joystick_state[i].button[j])
      {
        if(!(*block_button))
        {
          *button = true;
        }
        using_button = true;
        break;
      }
    }

    /* read sticks */
    for(j = 0; j < al_get_joystick_num_sticks(t3f_joystick[i]) && j <= 2; j++)
    {
      if(al_get_joystick_num_axes(t3f_joystick[i], j) > 0)
      {
        val = t3f_joystick_state[i].stick[j].axis[0];
        if(fabs(val) > 0.1)
        {
          if(!(*block_axes))
          {
            *x += (val - 0.1) / 0.9;
          }
          using_axis = true;
        }
      }
      if(al_get_joystick_num_axes(t3f_joystick[i], j) > 1)
      {
        val = t3f_joystick_state[i].stick[j].axis[1];
        if(fabs(val) > 0.1)
        {
          if(!(*block_axes))
          {
            *y += (val - 0.1) / 0.9;
          }
          using_axis = true;
        }
      }
    }
  }
  if(!using_axis)
  {
    *block_axes = false;
  }
  if(!using_button)
  {
    *block_button = false;
  }

  /* keep axis values within range */
  if(*x < -1.0)
  {
    *x = -1.0;
  }
  if(*x > 1.0)
  {
    *x = 1.0;
  }
  if(*y < -1.0)
  {
    *y = -1.0;
  }
  if(*y > 1.0)
  {
    *y = 1.0;
  }
}
