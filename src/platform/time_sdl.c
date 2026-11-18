#include "platform/time.h"
#include "utils/log.h"
#include "utils/msgbox.h"

#include <SDL.h>

bool time_init(void) {
    if(SDL_InitSubSystem(SDL_INIT_TIMER)) {
        err_msgbox("SDL2 timer initialization failed: %s", SDL_GetError());
        return false;
    }

    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    log_info("Found SDL v%d.%d.%d", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    log_info("Running on platform: %s", SDL_GetPlatform());

    return true;
}

void time_close(void) {
    SDL_QuitSubSystem(SDL_INIT_TIMER);
}

uint64_t time_ms(void) {
    return SDL_GetTicks64();
}

void time_sleep_ms(uint32_t milliseconds) {
    SDL_Delay(milliseconds);
}
