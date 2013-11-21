#include <SDL2/SDL.h>
#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "resources/sounds_loader.h"
#include "video/texture.h"
#include "video/video.h"
#include "game/text/languages.h"
#include "game/game_state.h"
#include "game/settings.h"
#include "game/ticktimer.h"
#include "console/console.h"

int _vsync = 0; // Needed in video.c
static int run = 0;
static int take_screenshot = 0;

int engine_init() {
    settings *setting = settings_get();
    
    int w = setting->video.screen_w;
    int h = setting->video.screen_h;
    int fs = setting->video.fullscreen;
    int vsync = setting->video.vsync;

    // Right now we only have one audio sink, so select that one.
    int sink_id = 0;

    // Initialize everything.
    _vsync = vsync;
    if(video_init(w, h, fs, vsync)) {
        goto exit_0;
    }
    if(audio_init(sink_id)) {
        goto exit_1;
    }
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
    DEBUG("Engine initialization successful.");
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
    audio_close();
exit_1:
    video_close();
exit_0:
    return 1;
}

void engine_run() {
    DEBUG(" --- BEGIN GAME LOG ---");

    // Set up game
    if(game_state_create()) {
        return;
    }

    // Game loop
    int frame_start = SDL_GetTicks();
    int omf_wait = 0;
    while(run && game_state_is_running()) {
        // Prepare rendering here
        video_render_prepare();
    
        // Handle events
        SDL_Event e;
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
            }
        
            // Console events
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_TAB) {
               if (console_window_is_open()) {
                   console_window_close();
               } else {
                   console_window_open();
               }
               continue;
            }
            if(console_window_is_open()) {
                 console_event(game_state_get_scene(), &e);
                 continue;
            }

            // Send events to scene (if active)
            if(!game_state_handle_event(&e)) {
                continue;
            }
        }

        // Render scene
        int dt = (SDL_GetTicks() - frame_start);
        omf_wait += dt;
        while(omf_wait > game_state_ms_per_tick()) {
            // Tick timers
            ticktimer_run();

            // Tick scene
            game_state_tick();

            // Tick console
            console_tick();
            
            // Handle waiting period leftover time
            omf_wait -= game_state_ms_per_tick();
        }
        frame_start = SDL_GetTicks();

        // Do the actual rendering jobs
        game_state_render();
        console_render();
        video_render_finish();
        audio_render();
        
        // If screenshot requested, do it here.
        if(take_screenshot) {
            image img;
            video_screenshot(&img);
            image_write_tga(&img, "screenshot.tga");
            image_free(&img);
            take_screenshot = 0;
        }
        
        // Delay stuff a bit if vsync is off
        if(!_vsync && run) {
            SDL_Delay(1);
        }
    }
    
    // Free scene object
    game_state_free();

    DEBUG(" --- END GAME LOG ---");
}

void engine_close() {
    ticktimer_close();
    console_close();
    altpals_close();
    fonts_close();
    lang_close();
    sounds_loader_close();
    audio_close();
    video_close();
    DEBUG("Engine deinit successful.");
}
