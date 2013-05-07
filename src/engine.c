#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "audio/soundloader.h"
#include "video/texture.h"
#include "video/texturelist.h"
#include "video/video.h"
#include "game/scene.h"
#include "game/settings.h"
#include "console/console.h"
#include <SDL2/SDL.h>

#define MS_PER_OMF_TICK 10

int run = 0;
int _vsync = 0;

settings _settings;
engine_global _engine_global = {NULL, &_settings};

int engine_init() {
    settings_init(&_settings);
    settings_load(&_settings);
    
    int w = _settings.video.screen_w;
    int h = _settings.video.screen_h;
    int fs = _settings.video.fullscreen;
    int vsync = _settings.video.vsync;
    _vsync = vsync;
    texturelist_init();
    if(video_init(w, h, fs, vsync)) {
        return 1;
    }
    if(audio_init()) {
        return 1;
    }
    if(soundloader_init("resources/SOUNDS.DAT")) {
        return 1;
    }
    if(console_init()) {
        return 1;
    }
    run = 1;
    return 0;
}

void engine_run() {
    DEBUG("Engine starting.");
    scene scene;
    scene.player1_har = NULL;
    scene.player2_har = NULL;
    scene.player1_ctrl = NULL;
    scene.player2_ctrl = NULL;
    
    engine_globals()->scene = &scene; 

    // Load scene
    if(scene_load(&scene, SCENE_INTRO)) {
        return;
    }

    // Game loop
    unsigned int scene_start = SDL_GetTicks();
    unsigned int omf_wait = 0;
    while(run) {
        // Prepare rendering here
        video_render_prepare();
    
        // We want to load another scene
        if(scene.this_id != scene.next_id) {
            if(scene.next_id == SCENE_NONE) {
                run = 0;
                break;
            }
            unsigned int nid = scene.next_id;
            scene_free(&scene);
            if(scene_load(&scene, nid)) {
                return;
            }
        }
        
        // Handle events
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            // Handle other events
            switch(e.type) {
                case SDL_QUIT:
                    run = 0;
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
                 console_event(&scene, &e);
                 continue;
            }

            // Send events to scene (if active)
            if(!scene_handle_event(&scene, &e)) {
                continue;
            }
        
            // Handle other events
            switch(e.type) {
            }
        }

        // Render scene
        int dt = SDL_GetTicks() - scene_start;
        omf_wait += dt;
        while(omf_wait > MS_PER_OMF_TICK) {
            scene_tick(&scene);
            console_tick();
            omf_wait -= MS_PER_OMF_TICK;
        }
        scene_start = SDL_GetTicks();

        // Do the actual rendering jobs
        scene_render(&scene);
        console_render();
        video_render_finish();
        audio_render(dt);
        
        // Delay stuff a bit if vsync is off
        if(!_vsync && run) {
            SDL_Delay(1);
        }
    }
    
    // Free scene object
    scene_free(&scene);
    
    DEBUG("Engine stopped.");
}

void engine_close() {
    console_close();
    video_close();
    texturelist_close();
    audio_close();
    soundloader_close();
    
    settings_save(&_settings);
    settings_free(&_settings);
}

engine_global *engine_globals() {
    return &_engine_global;
}
