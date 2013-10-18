#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "audio/soundloader.h"
#include "video/texture.h"
#include "video/video.h"
#include "game/text/languages.h"
#include "game/game_state.h"
#include "game/settings.h"
#include "console/console.h"
#include <SDL2/SDL.h>

int run = 0;
int _vsync = 0;
int take_screenshot = 0;

int engine_init() {
    settings *setting = settings_get();
    
    int w = setting->video.screen_w;
    int h = setting->video.screen_h;
    int fs = setting->video.fullscreen;
    int vsync = setting->video.vsync;

    _vsync = vsync;
    if(video_init(w, h, fs, vsync)) {
        return 1;
    }
    if(audio_init()) {
        return 1;
    }
    if(soundloader_init()) {
        return 1;
    }
    if(lang_init()) {
        return 1;
    }
    if(fonts_init()) {
        return 1;
    }
    if(altpals_init()) {
        return 1;
    }
    if(console_init()) {
        return 1;
    }
    run = 1;

    DEBUG("Engine initialization successful.");
    return 0;
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
        audio_render(dt);
        
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
    console_close();
    altpals_close();
    fonts_close();
    lang_close();
    soundloader_close();
    audio_close();
    video_close();
    DEBUG("Engine deinit successful.");
}
