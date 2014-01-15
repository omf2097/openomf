#include <string.h>
#include <strings.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <dumb/dumb.h>
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
#include "resources/ids.h"

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
    char *path = "";
    char *ip = NULL;
    int net_mode = NET_MODE_NONE;

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

#ifndef DEBUGMODE
    // where is the openomf binary, if this call fails we will look for resources in ./resources
    char *base_path = SDL_GetBasePath();
    char *resource_path = malloc(strlen(base_path) + 32);
    if(path != NULL) {
        const char *platform = SDL_GetPlatform();
        if (!strcasecmp(platform, "Windows")) {
            // on windows, the resources will be in ./resources, relative to the binary
            sprintf(resource_path, "%s%s", base_path, "resources\\");
            set_resource_path(resource_path);
        } else if (!strcasecmp(platform, "Linux")) {
            // on linux, the resources will be in ../share/openomf, relative to the binary
            // so if openomf is installed to /usr/local/bin, the resources will be in /usr/local/share/openomf
            sprintf(resource_path, "%s%s", base_path, "../share/openomf/");
            set_resource_path(resource_path);
        } else if (!strcasecmp(platform, "Mac OS X")) {
            // on OSX, GetBasePath returns the 'Resources' directory if run from an app bundle, so we can use this as-is
            sprintf(resource_path, "%s", base_path);
            set_resource_path(resource_path);
        }
        // any other platform will look in ./resources
        SDL_free(base_path);
    }
#endif

    // Config path
    char *config_path = malloc(strlen(path)+32);
    sprintf(config_path, "%s%s", path, "openomf.conf");

    // Logfile path
    char *logfile_path = malloc(strlen(path)+32);
    sprintf(logfile_path, "%s%s", path, "openomf.log");

    SDL_free(path);

    // Check arguments
    if(argc >= 2) {
        if(strcmp(argv[1], "-v") == 0) {
            printf("OpenOMF v0.6\n");
            printf("Source available at https://github.com/omf2097/ (MIT License)\n");
            printf("(C) 2097 Tuomas Virtanen, Andrew Thompson, Hunter and others\n");
            return 0;
        } else if(strcmp(argv[1], "-h") == 0) {
            printf("Arguments:\n");
            printf("-h       Prints this help\n");
            printf("-w       Writes a config file\n");
            printf("-c <ip>  Connect to server\n");
            printf("-l       Start server\n");
            return 0;
        } else if(strcmp(argv[1], "-w") == 0) {
            if(settings_write_defaults(config_path)) {
                fprintf(stderr, "Failed to write config file to '%s'!", config_path);
                return 1;
            } else {
                printf("Config file written to '%s'!", config_path);
            }
            return 0;
        } else if(strcmp(argv[1], "-c") == 0) {
            if(argc >= 3) {
                ip = strcpy(malloc(strlen(argv[2])+1), argv[2]);
            }
            net_mode = NET_MODE_CLIENT;
        } else if(strcmp(argv[1], "-l") == 0) {
            net_mode = NET_MODE_SERVER;
        }
    }

    // Init log
#if defined(DEBUGMODE) || defined(STANDALONE_SERVER)
    if(log_init(0)) {
        err_msgbox("Error while initializing log!");
        return 1;
    }
#else
    if(log_init(logfile_path)) {
        err_msgbox("Error while initializing log '%s'!", logfile_path);
        return 1;
    }
#endif

    // Random seed
    rand_seed(time(NULL));

    // Init stringparser
    sd_stringparser_lib_init();

    // Init config
    if(settings_init(config_path)) {
        err_msgbox("Failed to initialize settings file");
        goto exit_0;
    }
    settings_load();

    // Make sure the required resource files exist
    char missingfile[64];
    if(validate_resource_path(missingfile)) {
        err_msgbox("Resource file does not exist: %s", missingfile);
        goto exit_0;
    }

    if(ip) {
        settings_get()->net.net_server_ip = ip;
    }

    // Init SDL2
#ifdef STANDALONE_SERVER
    if(SDL_Init(SDL_INIT_TIMER)) {
#else
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)) {
#endif
        err_msgbox("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_1;
    }
    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    INFO("Found SDL v%d.%d.%d", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    INFO("Running on platform: %s", SDL_GetPlatform());

#ifndef STANDALONE_SERVER
    if(SDL_InitSubSystem(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC)) {
        err_msgbox("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_1;
    }
    INFO("Found %d joysticks attached", SDL_NumJoysticks());
    SDL_Joystick *joy;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        joy = SDL_JoystickOpen(0);
        if (joy) {
            INFO("Opened Joystick %d", i);
            INFO(" * Name:              %s", SDL_JoystickNameForIndex(i));
            INFO(" * Number of Axes:    %d", SDL_JoystickNumAxes(joy));
            INFO(" * Number of Buttons: %d", SDL_JoystickNumButtons(joy));
            INFO(" * Number of Balls:   %d", SDL_JoystickNumBalls(joy));
            INFO(" * Number of Hats:    %d", SDL_JoystickNumHats(joy));
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
        goto exit_2;
    }
    
    // Initialize engine
    if(engine_init()) {
        err_msgbox("Failed to initialize game engine.");
        goto exit_3;
    }
    
    // Run
    engine_run(net_mode);
    
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
    sd_stringparser_lib_deinit();
    INFO("Exit.");
    log_close();
    free(config_path);
    free(logfile_path);
#ifndef DEBUGMODE
    free(resource_path);
#endif
    return 0;
}
