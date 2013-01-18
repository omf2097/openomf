#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "audio/soundloader.h"
#include "video/texture.h"
#include "video/video.h"
#include "game/scene.h"
#include <SDL2/SDL.h>

#define MS_PER_OMF_TICK 10

int run = 0;
int _vsync = 0;

int engine_init() {
    int w = conf_int("screen_w");
    int h = conf_int("screen_h");
    int fs = conf_bool("fullscreen");
    int vsync = conf_bool("vsync");
    _vsync = vsync;

    if(video_init(w, h, fs, vsync)) {
        return 1;
    }
    if(audio_init()) {
        return 1;
    }
    if(soundloader_init("resources/SOUNDS.DAT")) {
        return 1;
    }
    run = 1;
    return 0;
}

void engine_run() {
    DEBUG("Engine starting.");
    scene scene;
    if(scene_load(&scene, SCENE_INTRO)) {
        return;
    }
    
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
            scene_load(&scene, nid);
        }
        
        // Handle events
        SDL_Event e;
        if(SDL_PollEvent(&e)) {
            // Send events to scene (if active)
            if(!scene_handle_event(&scene, &e)) {
                break;
            }
        
            // Handle other events
            switch(e.type) {
                case SDL_QUIT:
                    run = 0;
                    break;
            }
        }

        // Render scene
        int dt = SDL_GetTicks() - scene_start;
        omf_wait += dt;
        while(omf_wait > MS_PER_OMF_TICK) {
            scene_tick(&scene);
            omf_wait -= MS_PER_OMF_TICK;
        }
        scene_start = SDL_GetTicks();

        // Do the actual rendering jobs
        scene_render(&scene);
        video_render_finish();
        audio_render(dt);
        
        // Delay stuff a bit if vsync is off
        if(!_vsync && run) {
            SDL_Delay(1);
        }
    }
    
    scene_free(&scene);
    
    DEBUG("Engine stopped.");
}

void engine_close() {
    video_close();
    audio_close();
    soundloader_close();
}
