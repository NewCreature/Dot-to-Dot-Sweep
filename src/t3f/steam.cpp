#include "t3f/t3f.h"
#include "steam/steam_api_flat.h"

static bool _t3f_steam_integration_enabled = false;

bool t3f_init_steam_integration(void)
{
  #ifndef T3F_DISABLE_STEAM_INTEGRATION
    if(!SteamAPI_Init())
    {
      goto fail;
    }

    fail:
    {
      return false;
    }
  #else
    return false;
  #endif
}

void t3f_shutdown_steam_integration(void)
{
  #ifndef T3F_DISABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      SteamAPI_Shutdown();
      _t3f_steam_integration_enabled = false;
    }
  #endif
}

bool t3f_show_steam_text_input(int x, int y, int width, int height)
{
  #ifndef T3F_DISABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput(SteamUtils(), k_EFloatingGamepadTextInputModeModeSingleLine, x, y, width, height);
    }
  #endif
}
