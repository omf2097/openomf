#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "audio/soundloader.h"
#include "video/video.h"
#include "game/scene.h"
#include <SDL2/SDL.h>

int run;
int _vsync;

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
    unsigned int scene_delta = 0;
    unsigned int frame_period = (1/60.0f)*1000;
    unsigned int frame_start, frame_delta;
    int frame_wait;
    while(run) {
        frame_start = SDL_GetTicks();
    
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
                case SDL_KEYDOWN:
                    if(e.key.keysym.sym == SDLK_ESCAPE) {
                        run = 0;
                    }
                    break;
                case SDL_QUIT:
                    run = 0;
                    break;
            }
        }

        // Render scene
        scene_delta = SDL_GetTicks() - scene_start;
        scene_render(&scene, scene_delta);
        scene_start = SDL_GetTicks();

        // Do the actual rendering jobs
        video_render_finish();
        audio_render();
        if(!_vsync && run) {
            frame_delta = SDL_GetTicks() - frame_start;
            frame_wait = frame_period - frame_delta;
            SDL_Delay(frame_wait < 0 ? 0 : frame_wait);
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