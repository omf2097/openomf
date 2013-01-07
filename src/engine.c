#include "engine.h"

int engine_init() {
    if(video_init(320, 200, 32, 0, 1)) {
        return 1;
    }
    if(audio_init("AWESOEM")) {
        return 1;
    }
    return 0;
}

void engine_run() {

}

void engine_close() {
    video_close();
    audio_close();
}