#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "video/video.h"
#include <SDL2/SDL.h>

int run;

int engine_init() {
    int w = conf_int("screen_w");
    int h = conf_int("screen_h");
    int fs = conf_bool("fullscreen");
    int vsync = conf_bool("vsync");

    if(video_init(w, h, fs, vsync)) {
        return 1;
    }
    if(audio_init()) {
        return 1;
    }
    run = 1;
    return 0;
}

void engine_run() {
    DEBUG("Engine starting.");
    music_play("resources/MENU.PSM");
    while(run) {
        SDL_Event e;
        if(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                run = 0;
            }
        }
        
        audio_render();
        video_render();
        SDL_Delay(1);
    }
    DEBUG("Engine stopped.");
}

void engine_close() {
    video_close();
    audio_close();
}