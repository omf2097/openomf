#include "controller/game_controller_db.h"
#include "engine.h"
#include "game/game_state.h"
#include "game/utils/settings.h"
#include "game/utils/version.h"
#include "resources/ids.h"
#include "resources/pathmanager.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "utils/msgbox.h"
#include "utils/random.h"
#include <SDL.h>
#if defined(ARGTABLE2_FOUND)
#include <argtable2.h>
#elif defined(ARGTABLE3_FOUND)
#include <argtable3.h>
#endif
#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef SHA1_HASH
static const char *git_sha1_hash = "";
#else
static const char *git_sha1_hash = SHA1_HASH;
#endif

void scan_game_controllers(void) {
    log_info("Found %d joysticks attached", SDL_NumJoysticks());
    SDL_Joystick *joy;
    char guid_str[33];
    for(int i = 0; i < SDL_NumJoysticks(); i++) {
        joy = SDL_JoystickOpen(i);
        if(joy) {
            SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
            SDL_JoystickGetGUIDString(guid, guid_str, 33);
            log_info("Opened Joystick %d", i);
            log_info(" * Name:              %s", SDL_JoystickNameForIndex(i));
            log_info(" * Number of Axes:    %d", SDL_JoystickNumAxes(joy));
            log_info(" * Number of Buttons: %d", SDL_JoystickNumButtons(joy));
            log_info(" * Number of Balls:   %d", SDL_JoystickNumBalls(joy));
            log_info(" * Number of Hats:    %d", SDL_JoystickNumHats(joy));
            log_info(" * GUID          :    %s", guid_str);
        } else {
            log_info("Joystick %d is unsupported", i);
        }

        if(SDL_JoystickGetAttached(joy)) {
            SDL_JoystickClose(joy);
        }
    }
}

int main(int argc, char *argv[]) {
    // Set up initial state for misc things
    char *ip = NULL;
    char *lobbyaddr = NULL;
    char *oldlobbyaddr = NULL;
    char *trace_file = NULL;
    unsigned short connect_port = 0;
    unsigned short listen_port = 0;
    engine_init_flags init_flags;
    memset(&init_flags, 0, sizeof(init_flags));
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
    struct arg_lit *lobby = arg_lit0(NULL, "lobby", "Enter network game lobby");
    struct arg_str *lobbyarg =
        arg_str0(NULL, "lobby-addr", "<lobby server>", "Use <lobby server> as the lobby address to connect to");
    struct arg_str *connect = arg_str0("c", "connect", "<host>", "Connect to a remote game");
    struct arg_str *force_audio_backend =
        arg_str0(NULL, "force-audio-backend", "<force-audio-backend>", "Force an audio backend to use");
    struct arg_str *force_renderer = arg_str0(NULL, "force-renderer", "<force-renderer>", "Force a renderer to use");
    struct arg_str *trace = arg_str0("t", "trace", "<file>", "Trace netplay events to file");
    struct arg_int *port = arg_int0("p", "port", "<port>", "Port to connect or listen (default: 2097)");
    struct arg_file *play = arg_file0("P", "play", "<file>", "Play an existing recfile");
    struct arg_file *rec = arg_file0("R", "rec", "<file>", "Record a new recfile");
    struct arg_end *end = arg_end(30);
    void *argtable[] = {help,           vers,  listen, lobby, lobbyarg, connect, force_audio_backend,
                        force_renderer, trace, port,   play,  rec,      end};
    const char *progname = "openomf";

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
        printf("OpenOMF v%s\n", get_version_string());
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

    if(lobbyarg->count > 0) {
        lobbyaddr = omf_strdup(lobbyarg->sval[0]);
    }

    // Check other flags
    if(connect->count > 0) {
        init_flags.net_mode = NET_MODE_CLIENT;
        connect_port = 2097;
        ip = omf_strdup(connect->sval[0]);
        if(port->count > 0) {
            connect_port = port->ival[0] & 0xFFFF;
        }
    } else if(listen->count > 0) {
        init_flags.net_mode = NET_MODE_SERVER;
        listen_port = 2097;
    } else if(lobby->count > 0) {
        init_flags.net_mode = NET_MODE_LOBBY;
    } else if(play->count > 0) {
        init_flags.playback = 1;
        strncpy(init_flags.rec_file, play->filename[0], 254);
    } else if(rec->count > 0) {
        init_flags.record = 1;
        strncpy(init_flags.rec_file, rec->filename[0], 254);
    }
    if(force_renderer->count > 0) {
        strncpy_or_truncate(init_flags.force_renderer, force_renderer->sval[0], sizeof(init_flags.force_renderer));
    }
    if(force_audio_backend->count > 0) {
        strncpy_or_truncate(init_flags.force_audio_backend, force_audio_backend->sval[0],
                            sizeof(init_flags.force_audio_backend));
    }

    if(port->count > 0) {
        listen_port = port->ival[0] & 0xFFFF;
    }

    if(trace->count > 0) {
        trace_file = omf_strdup(trace->sval[0]);
    }

    // Init log
    log_init();
    log_add_file(pm_get_local_path(LOG_PATH), LOG_INFO);
#if defined(USE_COLORS)
    log_set_colors(true);
#else
    log_set_colors(false);
#endif
#if defined(DEBUGMODE)
    log_add_stderr(LOG_DEBUG, true);
    log_set_level(LOG_DEBUG);
#else
    log_set_level(LOG_INFO); // In release mode, drop debugs.
#endif

    // Simple header
    log_info("Starting OpenOMF v%s", get_version_string());
    if(strlen(git_sha1_hash) > 0) {
        log_info("Git SHA1 hash: %s", git_sha1_hash);
    }

    // Dump path manager log
    pm_log();

    // Random seed
    rand_seed(time(NULL));

    // Init config
    if(settings_init(pm_get_local_path(CONFIG_PATH))) {
        err_msgbox("Failed to initialize settings file");
        log_error("Failed to initialize settings file");
        goto exit_1;
    }
    settings_load();

    // Savegame directory check and create
    // TODO: Handle errors
    sg_init();

    // Network game override stuff
    if(ip) {
        log_debug("Connect IP overridden to %s", ip);
        omf_free(settings_get()->net.net_connect_ip);
        settings_get()->net.net_connect_ip = ip;
        // Set ip to NULL here since it will be freed by the settings.
        ip = NULL;
    }
    if(lobbyaddr) {
        log_debug("Lobby address overridden to %s", lobby);
        oldlobbyaddr = settings_get()->net.net_lobby_address;
        settings_get()->net.net_lobby_address = lobbyaddr;
    }

    if(trace_file) {
        omf_free(settings_get()->net.trace_file);
        settings_get()->net.trace_file = trace_file;
        trace_file = NULL;
    }
    if(connect_port > 0 && connect_port < 0xFFFF) {
        log_debug("Connect Port overridden to %u", connect_port & 0xFFFF);
        settings_get()->net.net_connect_port = connect_port;
    }
    if(listen_port > 0 && listen_port < 0xFFFF) {
        log_debug("Listen Port overridden to %u", listen_port & 0xFFFF);
        settings_get()->net.net_listen_port_start = listen_port;
    }

    // Init SDL2
    if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO)) {
        err_msgbox("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_2;
    }
    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    log_info("Found SDL v%d.%d.%d", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    log_info("Running on platform: %s", SDL_GetPlatform());

    if(SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC)) {
        err_msgbox("SDL2 Initialization failed: %s", SDL_GetError());
        goto exit_2;
    }

    // Load game controller support
    joystick_load_external_mappings();
    scan_game_controllers();

    // Init enet
    if(enet_initialize() != 0) {
        err_msgbox("Failed to initialize enet");
        goto exit_3;
    }

    // Initialize engine
    if(engine_init(&init_flags)) {
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
    if(oldlobbyaddr) {
        // we probably don't want to save this override
        settings_get()->net.net_lobby_address = oldlobbyaddr;
    }
    settings_save();
    settings_free();
exit_1:
    log_info("Exit.");
    log_close();
exit_0:
    if(ip) {
        omf_free(ip);
    }
    if(lobbyaddr) {
        omf_free(lobbyaddr);
    }
    if(trace_file) {
        omf_free(trace_file);
    }
    arg_freetable(argtable, N_ELEMENTS(argtable));
    pm_free();
    return ret;
}
