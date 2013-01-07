#include "engine.h"
#include <SDL2/SDL.h>

int run;

int engine_init() {
    if(video_init(640, 400, 0, 1)) {
        return 1;
    }
    if(audio_init("AWESOEM")) {
        return 1;
    }
    run = 1;
    return 0;
}

void engine_run() {
    while(run) {
        SDL_Event e;
        if(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                run = 0;
            }
        }
        
        video_render();
        SDL_Delay(1);
    }
}

void engine_close() {
    video_close();
    audio_close();
}