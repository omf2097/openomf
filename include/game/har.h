#ifndef _HAR_H
#define _HAR_H

#include <shadowdive/shadowdive.h>

typedef struct animationplayer_t animationplayer;

enum {
    ACT_KICK,
    ACT_PUNCH,
    ACT_UP,
    ACT_DOWN,
    ACT_LEFT,
    ACT_RIGHT,
    ACT_STOP,
};

typedef struct har_t har;

struct har_t {
    unsigned int x,y;
    sd_af_file *af;
    animationplayer *player;
};

void har_free(har *har);
int har_load(har *har, const char *filename); // Returns 0 on success
void har_tick(har *har); // Called by scene.c tick function at every game tick
void har_render(har *har); // Called by scene.h render function at every frame render
void har_act(har *har, int act_type); // Handle event passed from inputhandler

#endif // _HAR_H