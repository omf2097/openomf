#include <signal.h>
#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "resources/sounds_loader.h"
#include "video/texture.h"
#include "game/text/languages.h"
#include "game/game_state.h"
#include "game/settings.h"
#include "game/ticktimer.h"
#include "game/text/text.h"
#include "console/console.h"

int _vsync = 0; // Needed in video.c
static int run = 0;

void exit_handler(int s) {
    run = 0;
}

int engine_init() {
    // Initialize everything.
    if(sounds_loader_init()) {
        goto exit_2;
    }
    if(lang_init()) {
        goto exit_3;
    }
    if(fonts_init()) {
        goto exit_4;
    }
    if(altpals_init()) {
        goto exit_5;
    }
    if(console_init()) {
        goto exit_6;
    }
    ticktimer_init();
    
    // Return successfully
    run = 1;
    INFO("Engine initialization successful.");
    return 0;

    // If something failed, close in correct order
exit_6:
    altpals_close();
exit_5:
    fonts_close();
exit_4:
    lang_close();
exit_3:
    sounds_loader_close();
exit_2:
    return 1;
}

void engine_run() {
    INFO(" --- BEGIN GAME LOG ---");

    // Init interrupt signal handler
    signal (SIGINT, exit_handler);

    // Set up game
    game_state *gs = malloc(sizeof(game_state));
    if(game_state_create(gs)) {
        return;
    }

    // Game loop
    int frame_start = SDL_GetTicks();
    int omf_wait = 0;
    while(run && game_state_is_running(gs)) {    
        // Handle events
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            // Handle other events
            switch(e.type) {
                case SDL_QUIT:
                    run = 0;
                    break;
                case SDL_KEYDOWN:
                    break;
            }
        
            // Console events
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_TAB) {
                if(console_window_is_open()) {
                    console_window_close();
                } else {
                    console_window_open();
                }
                continue;
            }

            // If console windows is open, pass events to console. 
            // Otherwise to the objects. 
            if(console_window_is_open()) {
                console_event(game_state_get_scene(gs), &e);
            } else {
                game_state_handle_event(gs, &e);
            }
        }

        // Render scene
        int dt = (SDL_GetTicks() - frame_start);
        omf_wait += dt;
        while(omf_wait > game_state_ms_per_tick(gs)) {
            // Tick timers
            ticktimer_run();

            // Tick scene
            game_state_tick(gs);

            // Tick console
            console_tick();
            
            // Handle waiting period leftover time
            omf_wait -= game_state_ms_per_tick(gs);
        }
        frame_start = SDL_GetTicks();

        // Delay stuff a bit if vsync is off
        if(!_vsync && run) {
            SDL_Delay(1);
        }
    }
    
    // Free scene object
    game_state_free(gs);
    free(gs);

    INFO(" --- END GAME LOG ---");
}

void engine_close() {
    ticktimer_close();
    console_close();
    altpals_close();
    fonts_close();
    lang_close();
    sounds_loader_close();
    INFO("Engine deinit successful.");
}
