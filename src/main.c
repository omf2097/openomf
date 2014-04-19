#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <dumb.h>
#include <enet/enet.h>

#if defined(_WIN32) || defined(WIN32)
#include <shlobj.h> //SHCreateDirectoryEx
#endif

#include "engine.h"
#include "shadowdive/stringparser.h"
#include "utils/log.h"
#include "utils/random.h"
#include "game/game_state.h"
#include "game/settings.h"
#include "resources/global_paths.h"
#include "resources/ids.h"
#include "plugins/plugins.h"
#include "controller/gamecontrollerdb.h"

#ifdef STANDALONE_SERVER
void err_msgbox(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "... Exiting\n");
    va_end(args);
}
#else
char err_msgbox_buffer[1024];
void err_msgbox(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(err_msgbox_buffer, sizeof(err_msgbox_buffer), fmt, args);
    if(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", err_msgbox_buffer, NULL) != 0) {
        // if the message box failed, fallback to fprintf
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "... Exiting\n");
    }
    va_end(args);
}
#endif

int main(int argc, char *argv[]) {
    // Get path
    char *path = NULL;
    char *ip = NULL;
    unsigned short connect_port = 0;
    unsigned short listen_port = 0;
    int net_mode = NET_MODE_NONE;
    int portable_mode = 0;
    int ret = 0;

    // if openomf.conf exists in the current directory, switch to portable mode
    if(access("openomf.conf", F_OK) != -1) {
        // USB stick (portable) mode
        portable_mode = 1;
        path = SDL_malloc(1);
        path[0] = 0;
    } else {
        // Non-portable mode
        portable_mode = 0;
        path = SDL_GetPrefPath("AnanasGroup", "OpenOMF");
        if(path == NULL) {
            err_msgbox("Error getting config path: %s", SDL_GetError());
            return 1;
        }
#if defined(_WIN32) || defined(WIN32)
        // Ensure the path exists before continuing on
        // XXX shouldn't SDL_GetPrefPath automatically create the path if it doesn't exist?
        int sherr = SHCreateDirectoryEx(NULL, path, NULL);
        if(sherr == ERROR_FILE_EXISTS) {
            err_msgbox("Please delete this file and relaunch OpenOMF: %s", path);
            return 1;
        } else if(sherr != ERROR_SUCCESS && sherr != ERROR_ALREADY_EXISTS) {
            err_msgbox("Failed to create config path: %s", path);
            return 1;
        }
#endif
    }

    // Init some paths
    global_paths_init();
    global_path_build(CONFIG_PATH, path, "openomf.conf");
    global_path_build(LOG_PATH, path, "openomf.log");

    // Free SDL reserved path
    SDL_free(path);

    // Check arguments
    if(argc >= 2) {
        if(strcmp(argv[1], "-v") == 0) {
            printf("OpenOMF v0.6\n");
            printf("Source available at https://github.com/omf2097/ (MIT License)\n");
            printf("(C) 2097 Tuomas Virtanen, Andrew Thompson, Hunter and others\n");
            goto exit_0;
        } else if(strcmp(argv[1], "-h") == 0) {
            printf("Arguments:\n");
            printf("-h              Prints this help\n");
            printf("-w              Writes a config file\n");
            printf("-c [ip] [port]  Connect to server\n");
            printf("-l [port]       Start server\n");
            goto exit_0;
        } else if(strcmp(argv[1], "-w") == 0) {
            if(settings_write_defaults(global_path_get(CONFIG_PATH))) {
                fprintf(stderr, "Failed to write config file to '%s'!", global_path_get(CONFIG_PATH));
                ret = 1;
                goto exit_0;
            } else {
                printf("Config file written to '%s'!", global_path_get(CONFIG_PATH));
                goto exit_0;
            }
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
    if(log_init(global_path_get(LOG_PATH))) {
        err_msgbox("Error while initializing log '%s'!", global_path_get(LOG_PATH));
        goto exit_0;
    }
#endif

    // Random seed
    rand_seed(time(NULL));

    // Init stringparser
    sd_stringparser_lib_init();

    // Init config
    if(settings_init(global_path_get(CONFIG_PATH))) {
        err_msgbox("Failed to initialize settings file");
        goto exit_1;
    }
    settings_load();

    // Set default resource and plugins paths
    if(!strcasecmp(SDL_GetPlatform(), "Windows")) {
        global_path_set(RESOURCE_PATH, "resources\\");
        global_path_set(PLUGIN_PATH, "plugins\\");
    } else {
        global_path_set(RESOURCE_PATH, "resources/");
        global_path_set(PLUGIN_PATH, "plugins/");
    }

    // If we are not in debug mode, and not in portable mode,
    // set proper paths. If something fails, use the already set
    // defaults.
#ifndef DEBUGMODE
    if(!portable_mode) {
        // where is the openomf binary, if this call fails we will look for resources in ./resources
        char *base_path = SDL_GetBasePath();
        if(base_path != NULL) {
            if (!strcasecmp(SDL_GetPlatform(), "Windows")) {
                // on windows, the resources will be in ./resources, relative to the binary
                global_path_build(RESOURCE_PATH, base_path, "resources\\");
                global_path_build(PLUGIN_PATH, base_path, "plugins\\");
            } else if (!strcasecmp(SDL_GetPlatform(), "Linux")) {
                // on linux, the resources will be in ../share/openomf, relative to the binary
                // so if openomf is installed to /usr/local/bin, 
                // the resources will be in /usr/local/share/openomf
                global_path_build(RESOURCE_PATH, base_path, "../share/openomf/");
                global_path_set(PLUGIN_PATH, "/usr/lib/openomf/");
            } else if (!strcasecmp(SDL_GetPlatform(), "Mac OS X")) {
                // on OSX, GetBasePath returns the 'Resources' directory 
                // if run from an app bundle, so we can use this as-is
                global_path_set(RESOURCE_PATH, base_path);
                global_path_build(PLUGIN_PATH, base_path, "plugins/");
            }
            // any other platform will look in ./resources
            SDL_free(base_path);
        }
    }
#else
    // This disables warnings about unused variable
    (void)(portable_mode);
#endif

    // Print all paths if debug mode is on
    global_paths_print_debug();

    // Make sure the required resource files exist
    char *missingfile = NULL;
    if(validate_resource_path(&missingfile)) {
        err_msgbox("Resource file does not exist: %s", missingfile);
        free(missingfile);
        goto exit_1;
    }

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
    SDL_RWops *rw = SDL_RWFromConstMem(gamecontrollerdb, strlen(gamecontrollerdb));
    SDL_GameControllerAddMappingsFromRW(rw, 1);
    char *gamecontrollerdbpath = malloc(128);
    snprintf(gamecontrollerdbpath, 128, "%s/gamecontrollerdb.txt", global_path_get(RESOURCE_PATH));
    int mappings_loaded = SDL_GameControllerAddMappingsFromFile(gamecontrollerdbpath);
    if (mappings_loaded > 0) {
        DEBUG("loaded %d mappings from %s", mappings_loaded, gamecontrollerdbpath);
    }
    free(gamecontrollerdbpath);
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
    global_paths_close();
    return ret;
}
