#include <signal.h> // signal()
#include <SDL2/SDL.h>
#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "resources/sounds_loader.h"
#include "video/surface.h"
#include "video/video.h"
#include "video/tcache.h"
#include "game/text/languages.h"
#include "game/game_state.h"
#include "game/settings.h"
#include "game/ticktimer.h"
#include "game/text/text.h"
#include "console/console.h"

static int run = 0;
static int start_timeout = 30;
#ifndef STANDALONE_SERVER
static int take_screenshot = 0;
static int enable_screen_updates = 1;
#endif

void exit_handler(int s) {
    run = 0;
}

int engine_init() {
#ifndef STANDALONE_SERVER
    settings *setting = settings_get();
    
    int w = setting->video.screen_w;
    int h = setting->video.screen_h;
    int fs = setting->video.fullscreen;
    int vsync = setting->video.vsync;

    // Right now we only have one audio sink, so select that one.
    int sink_id = 0;

    // Initialize everything.
    tcache_init();
    if(video_init(w, h, fs, vsync)) {
        goto exit_0;
    }
    if(audio_init(sink_id)) {
        goto exit_1;
    }
#endif

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

#ifndef STANDALONE_SERVER
    audio_close();
exit_1:
    video_close();
exit_0:
    tcache_close();
#endif

    return 1;
}

void engine_run(int net_mode) {
    SDL_Event e;

    INFO(" --- BEGIN GAME LOG ---");

#ifdef STANDALONE_SERVER
    // Init interrupt signal handler
    signal(SIGINT, exit_handler);
#endif

#ifndef STANDALONE_SERVER
    // Game start timeout.
    // Wait a moment so that people are mentally prepared
    // (with the recording software on) for the game to start :)
    while(start_timeout > 0) {
        start_timeout--;
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                return;
            }
        }
        video_render_prepare();
        video_render_finish();
        continue;
    }

    // apply volume settings
    sound_set_volume(settings_get()->sound.sound_vol/10.0f);
#endif

    // Set up game
    game_state *gs = malloc(sizeof(game_state));
    if(game_state_create(gs, net_mode)) {
        return;
    }

    // Game loop
    int frame_start = SDL_GetTicks();
    int omf_wait = 0;
    while(run && game_state_is_running(gs)) {

#ifndef STANDALONE_SERVER
        // Handle events
        while(SDL_PollEvent(&e)) {
            // Handle other events
            switch(e.type) {
                case SDL_QUIT:
                    run = 0;
                    break;
                case SDL_KEYDOWN:
                    if(e.key.keysym.sym == SDLK_F1) {
                        take_screenshot = 1;
                    }
                    break;
                case SDL_WINDOWEVENT:
                    switch(e.window.event) {
                        case SDL_WINDOWEVENT_MINIMIZED:
                        case SDL_WINDOWEVENT_HIDDEN:
                            enable_screen_updates = 0;
                            DEBUG("Window hidden/minimized; screen rendering disabled.");
                            break;
                        case SDL_WINDOWEVENT_MAXIMIZED:
                        case SDL_WINDOWEVENT_RESTORED:
                        case SDL_WINDOWEVENT_EXPOSED:
                        case SDL_WINDOWEVENT_SHOWN:
                            enable_screen_updates = 1;
                            DEBUG("Window shown/exposed/maximized/restored; screen rendering enabled.");
                            break;
                    }
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
                console_event(gs, &e);
            } else {
                game_state_handle_event(gs, &e);
            }
        }
#endif

        // Render scene
        int dt = (SDL_GetTicks() - frame_start);
        omf_wait += dt;
        while(omf_wait > game_state_ms_per_tick(gs)) {
            // Tick scene
            game_state_tick(gs);

            // Tick console
            console_tick();

            // Handle cache
            tcache_tick();
            
            // Handle waiting period leftover time
            omf_wait -= game_state_ms_per_tick(gs);
        }
        frame_start = SDL_GetTicks();

#ifndef STANDALONE_SERVER
        // Handle audio
        audio_render();

        // Do the actual video rendering jobs
        if(enable_screen_updates) {

            video_render_prepare();
            game_state_render(gs);
            console_render();
            video_render_finish();
            
            // If screenshot requested, do it here.
            if(take_screenshot) {
                image img;
                video_screenshot(&img);
                image_write_tga(&img, "screenshot.tga");
                image_free(&img);
                take_screenshot = 0;
            }
        } else {
            // If screen updates are disabled, then wait until next global tick
            SDL_Delay(game_state_ms_per_tick(gs) - omf_wait);
        }
#else
        // In standalone, just wait for next global tick.
        SDL_Delay(game_state_ms_per_tick(gs) - omf_wait);
#endif // STANDALONE_SERVER
    }
    
    // Free scene object
    game_state_free(gs);
    free(gs);

    INFO(" --- END GAME LOG ---");
}

void engine_close() {
    console_close();
    altpals_close();
    fonts_close();
    lang_close();
    sounds_loader_close();
#ifndef STANDALONE_SERVER
    audio_close();
    video_close();
    tcache_close();
#endif
    INFO("Engine deinit successful.");
}
