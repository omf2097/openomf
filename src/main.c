#include "engine.h"
#include <SDL2/SDL.h>
#include <dumb/dumb.h>

int main(int argc, char *argv[]) {
    // Init libDumb
    dumb_register_stdfiles();

    // Init SDL2
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)) {
        printf("[E] SDL2 Initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    printf("[D] Found SDL v%d.%d.%d\n", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    
    // Initialize engine
    if(engine_init()) {
        return 1;
    }
    
    // Run
    engine_run();
    
    // Close everything
    engine_close();
    SDL_Quit();
    return 0;
}