#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <dumb.h>
#include <enet/enet.h>
#include <shadowdive/stringparser.h>
#include "engine.h"
#include "utils/log.h"
#include "utils/random.h"
#include "utils/msgbox.h"
#include "game/game_state.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "resources/ids.h"
#include "plugins/plugins.h"
#include "controller/gamecontrollerdb.h"

int main(int argc, char *argv[]) {
    // Get path
    char *ip = NULL;
    unsigned short connect_port = 0;
    unsigned short listen_port = 0;
    int net_mode = NET_MODE_NONE;
    int ret = 0;

    // Path manager
    if(pm_init() != 0) {
        err_msgbox(pm_get_errormsg());
        return 1;
    }

    // Check arguments
    if(argc >= 2) {
        if(strcmp(argv[1], "-v") == 0) {
            printf("OpenOMF v%d.%d.%d\n", V_MAJOR, V_MINOR, V_PATCH);
            printf("Source available at https://github.com/omf2097/ under MIT License\n");
            printf("(C) 2097 Tuomas Virtanen, Andrew Thompson, Hunter and others\n");
            goto exit_0;
        } else if(strcmp(argv[1], "-h") == 0) {
            printf("Arguments:\n");
            printf("-h              Prints this help\n");
            printf("-c [ip] [port]  Connect to server\n");
            printf("-l [port]       Start server\n");
            goto exit_0;
        } else if(strcmp(argv[1], "-c") == 0) {
            if(argc >= 3) {
                ip = strcpy(malloc(strlen(argv[2])+1), argv[2]);
            }
            if(argc >= 4) {
                connect_port = atoi(argv[3]);
            }
            net_mode = NET_MODE_CLIENT;
        } else if(strcmp(argv[1], "-l") == 0) {
            if(argc >= 3) {
                listen_port = atoi(argv[2]);
            }
            net_mode = NET_MODE_SERVER;
        }
    }

    // Init log
#if defined(DEBUGMODE) || defined(STANDALONE_SERVER)
    if(log_init(0)) {
        err_msgbox("Error while initializing log!");
        goto exit_0;
    }
#else
    if(log_init(pm_get_local_path(LOG_PATH))) {
        err_msgbox("Error while initializing log '%s'!", pm_get_local_path(LOG_PATH));
        goto exit_0;
    }
#endif

    // Simple header
    INFO("Starting OpenOMF v%d.%d.%d", V_MAJOR, V_MINOR, V_PATCH);

    // Dump pathmanager log
    pm_log();

    // Random seed
    rand_seed(time(NULL));

    // Init stringparser
    sd_stringparser_lib_init();

    // Init config
    if(settings_init(pm_get_local_path(CONFIG_PATH))) {
        err_msgbox("Failed to initialize settings file");
        goto exit_1;
    }
    settings_load();

    // Find plugins and make sure they are valid
    plugins_init();

    // Network game override stuff
    if(ip) {
        DEBUG("Connect IP overridden to %s", ip);
        settings_get()->net.net_connect_ip = ip;
    }
    if(connect_port > 0 && connect_port < 0xFFFF) {
        DEBUG("Connect Port overridden to %u", connect_port&0xFFFF);
        settings_get()->net.net_connect_port = connect_port;
    }
    if(listen_port > 0 && listen_port < 0xFFFF) {
        DEBUG("Listen Port overridden to %u", listen_port&0xFFFF);
        settings_get()->net.net_listen_port = listen_port;
    }

    // Init SDL2
    unsigned int sdl_flags = SDL_INIT_TIMER;
#ifndef STANDALONE_SERVER
    sdl_flags |= SDL_INIT_VIDEO;
#endif
    if(SDL_Init(sdl_flags)) {
        err_msgbox("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_2;
    }
    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    INFO("Found SDL v%d.%d.%d", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    INFO("Running on platform: %s", SDL_GetPlatform());

#ifndef STANDALONE_SERVER
    if(SDL_InitSubSystem(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC)) {
        err_msgbox("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_2;
    }

    // Attempt to find gamecontrollerdb.txt, either from resources or from
    // built-in header
    SDL_RWops *rw = SDL_RWFromConstMem(gamecontrollerdb, strlen(gamecontrollerdb));
    SDL_GameControllerAddMappingsFromRW(rw, 1);
    char *gamecontrollerdbpath = malloc(128);
    snprintf(gamecontrollerdbpath, 128, "%s/gamecontrollerdb.txt", pm_get_local_path(RESOURCE_PATH));
    int mappings_loaded = SDL_GameControllerAddMappingsFromFile(gamecontrollerdbpath);
    if (mappings_loaded > 0) {
        DEBUG("loaded %d mappings from %s", mappings_loaded, gamecontrollerdbpath);
    }
    free(gamecontrollerdbpath);

    // Load up joysticks
    INFO("Found %d joysticks attached", SDL_NumJoysticks());
    SDL_Joystick *joy;
    char guidstr[33];
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        joy = SDL_JoystickOpen(i);
        if (joy) {
            SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
            SDL_JoystickGetGUIDString(guid, guidstr, 33);
            INFO("Opened Joystick %d", i);
            INFO(" * Name:              %s", SDL_JoystickNameForIndex(i));
            INFO(" * Number of Axes:    %d", SDL_JoystickNumAxes(joy));
            INFO(" * Number of Buttons: %d", SDL_JoystickNumButtons(joy));
            INFO(" * Number of Balls:   %d", SDL_JoystickNumBalls(joy));
            INFO(" * Number of Hats:    %d", SDL_JoystickNumHats(joy));
            INFO(" * GUID          :    %s", guidstr);
        } else {
            INFO("Joystick %d is unsupported", i);
        }

        if (SDL_JoystickGetAttached(joy)) {
            SDL_JoystickClose(joy);
        }
    }

    // Init libDumb
    dumb_register_stdfiles();
#endif

    // Init enet
    if(enet_initialize() != 0) {
        err_msgbox("Failed to initialize enet");
        goto exit_3;
    }

    // Initialize engine
    if(engine_init()) {
        err_msgbox("Failed to initialize game engine.");
        goto exit_4;
    }

    // Run
    engine_run(net_mode);

    // Close everything
    engine_close();
exit_4:
    enet_deinitialize();
exit_3:
    SDL_Quit();
exit_2:
    dumb_exit();
    settings_save();
    settings_free();
exit_1:
    sd_stringparser_lib_deinit();
    INFO("Exit.");
    log_close();
exit_0:
    if (ip) {
        free(ip);
    }
    plugins_close();
    pm_free();
    return ret;
}
