#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "audio/soundloader.h"
#include "video/video.h"
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
    while(run) {
        SDL_Event e;
        if(SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_KEYDOWN:
                if(e.key.keysym.sym == SDLK_a) {
                    if(music_playing()) {
                        DEBUG("Already playing - Stop first.");                
                    } else {
                        DEBUG("Attempting to play resources/MENU.PSM");
                        music_play("resources/MENU.PSM");
                    }
                }
                if(e.key.keysym.sym == SDLK_s) {
                    if(music_playing()) {
                        DEBUG("Already playing - Stop first.");                
                    } else {
                        DEBUG("Attempting to play resources/END.PSM");
                        music_play("resources/END.PSM");
                    }
                }
                if(e.key.keysym.sym == SDLK_d) {
                    music_stop();
                }
                if(e.key.keysym.sym == SDLK_q) { soundloader_play(10); }
                if(e.key.keysym.sym == SDLK_w) { soundloader_play(11); }
                if(e.key.keysym.sym == SDLK_e) { soundloader_play(12); }
                if(e.key.keysym.sym == SDLK_r) { soundloader_play(13); }
                if(e.key.keysym.sym == SDLK_t) { soundloader_play(14); }
                if(e.key.keysym.sym == SDLK_y) { soundloader_play(15); }
                if(e.key.keysym.sym == SDLK_ESCAPE) {
                    run = 0;
                }
                break;
                
            case SDL_QUIT:
                run = 0;
                break;
            }
        }
        
        video_render_prepare();
        
        video_render_finish();
        audio_render();
        if(!_vsync) {
            SDL_Delay(5);
        }
    }
    DEBUG("Engine stopped.");
}

void engine_close() {
    video_close();
    audio_close();
    soundloader_close();
}