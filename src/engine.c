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
#include "game/text/languages.h"
#include "game/scene.h"
#include "game/settings.h"
#include "game/physics/space.h"
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
    texturelist_init();
    if(video_init(w, h, fs, vsync)) {
        return 1;
    }
    if(lang_init()) {
        return 1;
    }
    if(fonts_init()) {
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
    physics_space_init();
    physics_space_set_gravity(10.0f);
    run = 1;
    return 0;
}

void engine_run() {
    DEBUG("Engine starting.");
    scene scene;
    scene.player1.har = NULL;
    scene.player2.har = NULL;
    scene.player1.ctrl = NULL;
    scene.player2.ctrl = NULL;

    float t = 0.0f;
    
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
                case SDL_KEYDOWN:
                    if(e.key.keysym.sym == SDLK_F1) {
                        take_screenshot = 1;
                    }
                    break;
            }
        }

        // Render scene
        int dt = SDL_GetTicks() - scene_start;
        omf_wait += dt;
        while(omf_wait > scene_ms_per_tick(&scene)) {
            // Tick physics engine
            t = ((float)scene_ms_per_tick(&scene)) / 1000;
            physics_space_tick(t);
            
            // Tick scene
            scene_tick(&scene);
            
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
            
            // Tick console
            console_tick();
            
            // Handle waiting period leftover time
            omf_wait -= scene_ms_per_tick(&scene);
        }
        scene_start = SDL_GetTicks();

        // Do the actual rendering jobs
        scene_render(&scene);
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
    scene_free(&scene);
    
    DEBUG("Engine stopped.");
}

void engine_close() {
    physics_space_close();
    console_close();
    fonts_close();
    lang_close();
    video_close();
    texturelist_close();
    audio_close();
    soundloader_close();
}
