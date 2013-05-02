#include "game/har.h"
#include "utils/log.h"
#include "video/texture.h"
#include "video/video.h"
#include "audio/sound.h"
#include "game/animation.h"
#include "game/animationplayer.h"


int har_load(har *h, const char *file) {
    h->x = 160;
    h->y = 100;
    h->af = sd_af_create();
    return !sd_af_load(h->af, file);
}

void har_free(har *h) {
    sd_af_delete(h->af);
}

void har_tick(har *har) {

}

void har_render(har *har) {

}

void har_act(har *har, int act_type) {

}