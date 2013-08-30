#include "engine.h"
#include "utils/log.h"
#include "game/settings.h"
#include <SDL2/SDL.h>
#include <dumb/dumb.h>
#include <enet/enet.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Check arguments
    if(argc >= 2) {
        if(strcmp(argv[1], "-v") == 0) {
            printf("OpenOMF v0.1\n");
            printf("Source available at https://github.com/omf2097/ (MIT License)\n");
            printf("(C) 2013 Tuomas Virtanen\n");
            return 0;
        } else if(strcmp(argv[1], "-h") == 0) {
            printf("Arguments:\n");
            printf("-h      Prints this help\n");
            printf("-w      Writes a config file\n");
            return 0;
        } else if(strcmp(argv[1], "-w") == 0) {
            if(settings_write_defaults()) {
                printf("Failed to write config file!\n");
                return 1;
            } else {
                printf("Config file written to 'openomf.conf'!\n");
            }
            return 0;
        }
    }

    // Init log
    if(log_init(0)) {
        printf("Error while initializing log!\n");
        return 1;
    }
    
    // Init config
    if(settings_init()) {
        goto exit_0;
    }
    settings_load();
    
    // Init libDumb
    dumb_register_stdfiles();
    
    // Init SDL2
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)) {
        PERROR("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_1;
    }
    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    DEBUG("Found SDL v%d.%d.%d", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    
    // Init enet
    if(enet_initialize() != 0) {
        PERROR("Failed to initialize enet");
        goto exit_2;
    }
    
    // Initialize engine
    if(engine_init()) {
        PERROR("Failed to initialize game engine.");
        goto exit_3;
    }
    
    // Run
    engine_run();
    
    // Close everything
    engine_close();
exit_3:
    enet_deinitialize();
exit_2:
    SDL_Quit();
exit_1:
    dumb_exit();
    settings_save();
    settings_free();
exit_0:
    DEBUG("Exit.");
    log_close();
    return 0;
}
