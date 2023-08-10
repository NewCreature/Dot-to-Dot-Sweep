#include "t3f/t3f.h"
#include "steam/steam_api_flat.h"

#define T3F_STEAM_STORE_STATE_NONE        0
#define T3F_STEAM_STORE_STATE_IN_PROGRESS 1
#define T3F_STEAM_STORE_STATE_DONE        2
#define T3F_STEAM_STORE_STATE_ERROR       3

#define T3F_STEAM_STATS_STORE_INTERVAL 10.0

class _t3f_steam_stats_class
{
  private:
	  STEAM_CALLBACK(_t3f_steam_stats_class, OnUserStatsReceived, UserStatsReceived_t);
	  STEAM_CALLBACK(_t3f_steam_stats_class, OnUserStatsStored, UserStatsStored_t);
};

static bool _t3f_steam_integration_enabled = false;
static bool _t3f_steam_stats_ready = false;
static bool _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_NONE;
static _t3f_steam_stats_class * _t3f_steam_stats = NULL;
static double _t3f_steam_store_time = 0.0;

void _t3f_steam_stats_class::OnUserStatsReceived(UserStatsReceived_t * callback)
{
	if(callback->m_eResult == k_EResultOK)
  {
    _t3f_steam_stats_ready = true;
  }
}

void _t3f_steam_stats_class::OnUserStatsStored(UserStatsStored_t * callback)
{
	if(callback->m_eResult == k_EResultOK)
  {
    _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_DONE;
    al_remove_config_key(t3f_user_data, "Achievements", "Stored");
  }
  else
  {
    _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_ERROR;
  }
}

bool t3f_init_steam_integration(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(!SteamAPI_Init())
    {
      goto fail;
    }
    _t3f_steam_integration_enabled = true;
    _t3f_steam_stats = new _t3f_steam_stats_class;
    if(_t3f_steam_stats == NULL)
    {
      goto fail;
    }
    SteamAPI_ISteamUserStats_RequestCurrentStats(SteamUserStats());
    return true;

    fail:
    {
      t3f_shutdown_steam_integration();
      return false;
    }
  #else
    return false;
  #endif
}

void t3f_shutdown_steam_integration(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      if(_t3f_steam_stats)
      {
        delete _t3f_steam_stats;
      }
      SteamAPI_Shutdown();
      _t3f_steam_integration_enabled = false;
    }
  #endif
}

bool t3f_show_steam_text_input(int x, int y, int width, int height)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput(SteamUtils(), k_EFloatingGamepadTextInputModeModeSingleLine, x, y, width, height);
    }
  #endif
  return true;
}

bool t3f_synchronize_achievements_with_steam(T3F_ACHIEVEMENTS_LIST * achievements_list)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    int i;

    if(_t3f_steam_integration_enabled && _t3f_steam_stats_ready)
    {
      for(i = 0; i < achievements_list->entries; i++)
      {
        if(achievements_list->entry[i].step >= achievements_list->entry[i].steps)
        {
          SteamAPI_ISteamUserStats_SetAchievement(SteamUserStats(), achievements_list->entry[i].steam_id);
        }
      }
      SteamAPI_ISteamUserStats_StoreStats(SteamUserStats());
      _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_IN_PROGRESS;
      al_set_config_value(t3f_user_data, "Achievements", "Stored", "false");
      return true;
    }
  #endif
  return false;
}

const char * t3f_get_steam_user_display_name(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      return SteamAPI_ISteamFriends_GetPersonaName(SteamFriends());
    }
  #endif
  return NULL;
}

void t3f_steam_integration_logic(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    const char * val;

    if(_t3f_steam_integration_enabled)
    {
      SteamAPI_ISteamNetworkingSockets_RunCallbacks(SteamNetworkingSockets());

      /* attempt to store current achievements if it is needed and store isn't currently processing */
      if(_t3f_steam_stats_store_state != T3F_STEAM_STORE_STATE_IN_PROGRESS)
      {
        val = al_get_config_value(t3f_user_data, "Achievements", "Stored");
        if(val && !strcmp(val, "false"))
        {
          if(al_get_time() - _t3f_steam_store_time >= T3F_STEAM_STATS_STORE_INTERVAL)
          {
            SteamAPI_ISteamUserStats_StoreStats(SteamUserStats());
            _t3f_steam_store_time = al_get_time();
            _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_IN_PROGRESS;
          }
        }
      }
    }
  #endif
}
