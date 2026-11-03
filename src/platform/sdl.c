#include "controller/game_controller_db.h"
#include "platform/platform.h"
#include "utils/log.h"
#include "utils/msgbox.h"

#include <SDL.h>

static void scan_game_controllers(void) {
    log_info("Found %d joysticks attached", SDL_NumJoysticks());
    char guid_str[33];
    for(int i = 0; i < SDL_NumJoysticks(); i++) {
        SDL_Joystick *joy = SDL_JoystickOpen(i);
        if(joy) {
            SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
            SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
            log_info("Opened Joystick %d", i);
            log_info(" * Name:              %s", SDL_JoystickNameForIndex(i));
            log_info(" * Number of Axes:    %d", SDL_JoystickNumAxes(joy));
            log_info(" * Number of Buttons: %d", SDL_JoystickNumButtons(joy));
            log_info(" * Number of Balls:   %d", SDL_JoystickNumBalls(joy));
            log_info(" * Number of Hats:    %d", SDL_JoystickNumHats(joy));
            log_info(" * GUID          :    %s", guid_str);
            SDL_JoystickClose(joy);
        } else {
            log_info("Joystick %d is unsupported", i);
        }
    }
}

bool platform_init(void) {
    if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO)) {
        err_msgbox("SDL2 Initialization failed: %s", SDL_GetError());
        return false;
    }

    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    log_info("Found SDL v%d.%d.%d", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    log_info("Running on platform: %s", SDL_GetPlatform());

    if(SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER)) {
        err_msgbox("Joystick initialization failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    if(SDL_InitSubSystem(SDL_INIT_HAPTIC)) {
        log_warn("Haptic subsystem failed to initialize -- rumble will be disabled.");
    }

    joystick_load_external_mappings();
    scan_game_controllers();
    return true;
}

void platform_close(void) {
    SDL_Quit();
}

uint64_t platform_time_ms(void) {
    return SDL_GetTicks64();
}

void platform_sleep_ms(uint32_t milliseconds) {
    SDL_Delay(milliseconds);
}
