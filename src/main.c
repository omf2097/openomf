#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include <SDL2/SDL.h>
#include <dumb/dumb.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Check arguments
    if(argc >= 2) {
        if(strcmp(argv[1], "-v") == 0) {
            printf("OpenOMF v0.1\n");
            printf("Source available at https://github.com/omf2097/ (MIT License)\n");
            printf("(C) 2013 Tuomas Virtanen\n");
            return 0;
        }
    }

    // Init log
    if(log_init(0)) {
        printf("Error while initializing log!\n");
        return 1;
    }
    
    // Init config
    if(conf_init("openomf.cfg")) {
        ERROR("Error while attempting to open configuration file 'openomf.cfg'!");
        goto exit_0;
    }
    
    // Init libDumb
    dumb_register_stdfiles();
    
    // Init SDL2
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)) {
        ERROR("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_1;
    }
    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    DEBUG("Found SDL v%d.%d.%d", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    
    // Initialize engine
    if(engine_init()) {
        goto exit_2;
    }
    
    // Run
    engine_run();
    
    // Close everything
    engine_close();
exit_2:
    SDL_Quit();
exit_1:
    dumb_exit();
    conf_close();
exit_0:
    DEBUG("Exit.");
    log_close();
    return 0;
}