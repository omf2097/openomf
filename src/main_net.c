#include <stdio.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <enet/enet.h>

#include "engine.h"
#include "shadowdive/stringparser.h"
#include "utils/log.h"
#include "utils/random.h"
#include "game/settings.h"

int main(int argc, char *argv[]) {
    // Get path

    char *path = "";

    // Disable SDL_GetPrefPath for now, it seems to be somewhat buggy
/*
    char *path = SDL_GetPrefPath("AnanasGroup", "OpenOMF");
    if(path == NULL) {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }
*/

    // Config path
    char config_path[strlen(path)+32];
    sprintf(config_path, "%s%s", path, "openomf.conf");

    // Logfile path
    char logfile_path[strlen(path)+32];
    sprintf(logfile_path, "%s%s", path, "openomf.log");

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
            if(settings_write_defaults(config_path)) {
                fprintf(stderr, "Failed to write config file to '%s'!\n", config_path);
                fflush(stderr);
                return 1;
            } else {
                printf("Config file written to '%s'!\n", config_path);
            }
            return 0;
        }
    }

    // Init log
    if(log_init(0)) {
        fprintf(stderr, "Error while initializing log!\n");
        fflush(stderr);
        return 1;
    }

    // Random seed
    rand_seed(time(NULL));

    // Init stringparser
    sd_stringparser_lib_init();

    // Init config
    if(settings_init(config_path)) {
        goto exit_0;
    }
    settings_load();

    // Init SDL2
    if(SDL_Init(SDL_INIT_EVENTS|SDL_INIT_TIMER)) {
        PERROR("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_1;
    }
    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    INFO("Found SDL v%d.%d.%d", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    INFO("Running on platform: %s", SDL_GetPlatform());

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
    settings_save();
    settings_free();
exit_0:
    sd_stringparser_lib_deinit();
    INFO("Exit.");
    log_close();
#ifndef DEBUGMODE
    /*
    SDL_free(path);
    */
#endif
    return 0;
}
