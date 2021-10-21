#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <SDL.h>
#include <argtable2.h>
#ifdef USE_DUMB
#include <dumb.h>
#endif
#include <enet/enet.h>
#include "engine.h"
#include "utils/log.h"
#include "utils/random.h"
#include "utils/msgbox.h"
#include "game/game_state.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "resources/ids.h"
#include "resources/sgmanager.h"
#include "plugins/plugins.h"
#include "controller/gamecontrollerdb.h"
#include "utils/allocator.h"
#include "utils/compat.h"

#ifndef SHA1_HASH
    const char *git_sha1_hash = "";
#else
    const char *git_sha1_hash = SHA1_HASH;
#endif

int main(int argc, char *argv[]) {
    // Set up initial state for misc things
    char *ip = NULL;
    unsigned short connect_port = 0;
    unsigned short listen_port = 0;
    engine_init_flags init_flags;
    init_flags.net_mode = NET_MODE_NONE;
    init_flags.record = 0;
    memset(init_flags.rec_file, 0, 255);
    int ret = 0;

    // Path manager
    if(pm_init() != 0) {
        err_msgbox(pm_get_errormsg());
        fprintf(stderr, "Error: %s.\n", pm_get_errormsg());
        return 1;
    }

    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_lit *listen = arg_lit0("l", "listen", "Start a network game server");
    struct arg_str *connect = arg_str0("c", "connect", "<host>", "Connect to a remote game");
    struct arg_int *port = arg_int0("p", "port", "<port>","Port to connect or listen (default: 2097)");
    struct arg_file *play = arg_file0("P", "play", "<file>", "Play an existing recfile");
    struct arg_file *rec = arg_file0("R", "rec", "<file>", "Record a new recfile");
    struct arg_end *end = arg_end(30);
    void* argtable[] = {help, vers, listen, connect, port, play, rec, end};
    const char* progname = "openomf";

    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        fprintf(stderr, "Error: insufficient memory\n");
        err_msgbox("Error: insufficient memory");
        return 1;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help & version
    if(help->count > 0) {
        fprintf(stderr, "Usage: %s", progname);
        arg_print_syntax(stderr, argtable, "\n");
        fprintf(stderr, "\nArguments:\n");
        arg_print_glossary(stderr, argtable, "%-25s %s\n");
        goto exit_0;
    }
    if(vers->count > 0) {
        printf("OpenOMF v%d.%d.%d\n", V_MAJOR, V_MINOR, V_PATCH);
        printf("Source available at https://github.com/omf2097/ under MIT License\n");
        printf("(C) 2097 Tuomas Virtanen, Andrew Thompson, Hunter and others\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stderr, end, progname);
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Check other flags
    if(connect->count > 0) {
        init_flags.net_mode = NET_MODE_CLIENT;
        connect_port = 2097;
        ip = strdup(connect->sval[0]);
        if(port->count > 0) {
            connect_port = port->ival[0] & 0xFFFF;
        }
    }
    else if(listen->count > 0) {
        init_flags.net_mode = NET_MODE_SERVER;
        listen_port = 2097;
        if(port->count > 0) {
            listen_port = port->ival[0] & 0xFFFF;
        }
    }
    else if(play->count > 0) {
        strncpy(init_flags.rec_file, play->filename[0], 254);
    }
    else if(rec->count > 0) {
        init_flags.record = 1;
        strncpy(init_flags.rec_file, rec->filename[0], 254);
    }

    // Init log
#if defined(DEBUGMODE) || defined(STANDALONE_SERVER)
    if(log_init(0)) {
        err_msgbox("Error while initializing log!");
        printf("Error while initializing log!\n");
        goto exit_0;
    }
#else
    if(log_init(pm_get_local_path(LOG_PATH))) {
        err_msgbox("Error while initializing log '%s'!", pm_get_local_path(LOG_PATH));
        printf("Error while initializing log '%s'!", pm_get_local_path(LOG_PATH));
        goto exit_0;
    }
#endif

    // Simple header
    INFO("Starting OpenOMF v%d.%d.%d", V_MAJOR, V_MINOR, V_PATCH);
    if(strlen(git_sha1_hash) > 0) {
        INFO("Git SHA1 hash: %s", git_sha1_hash);
    }

    // Dump pathmanager log
    pm_log();

    // Random seed
    rand_seed(time(NULL));

    // Init config
    if(settings_init(pm_get_local_path(CONFIG_PATH))) {
        err_msgbox("Failed to initialize settings file");
        PERROR("Failed to initialize settings file");
        goto exit_1;
    }
    settings_load();

    // Savegame directory check and create
    // TODO: Handle errors
    sg_init();

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
    char *gamecontrollerdbpath = omf_calloc(128, 1);
    snprintf(gamecontrollerdbpath, 128, "%s/gamecontrollerdb.txt", pm_get_local_path(RESOURCE_PATH));
    int mappings_loaded = SDL_GameControllerAddMappingsFromFile(gamecontrollerdbpath);
    if (mappings_loaded > 0) {
        DEBUG("loaded %d mappings from %s", mappings_loaded, gamecontrollerdbpath);
    }
    omf_free(gamecontrollerdbpath);

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
#ifdef USE_DUMB
    dumb_register_stdfiles();
#endif // USE_DUMB

#endif // STANDALONE_SERVER

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
    engine_run(&init_flags);

    // Close everything
    engine_close();
exit_4:
    enet_deinitialize();
exit_3:
    SDL_Quit();
exit_2:
#ifdef USE_DUMB
    dumb_exit();
#endif
    settings_save();
    settings_free();
exit_1:
    INFO("Exit.");
    log_close();
exit_0:
    if(ip) {
        omf_free(ip);
    }
    plugins_close();
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    pm_free();
    return ret;
}
