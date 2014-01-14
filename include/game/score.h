#ifndef _SCORE_H
#define _SCORE_H

#include <string.h>
#include <stdlib.h>
#include "utils/list.h"
#include "video/surface.h"
#include "game/protos/object.h"
#include "game/text/text.h"

enum {
    SCORE_EV_PUNCH = 0,
    SCORE_EV_KICK,
    SCORE_EV_ROUNDHOUSE,
    SCORE_EV_PERFECTROUND,
};

typedef struct chr_score_t {
    int score;
    int x,y;
    int direction;
    float multiplier;
    list texts;

    int consecutive_hits;
    int consecutive_hit_score;
    int combo_hits;
    int combo_hit_score;
} chr_score;

void chr_score_create(chr_score *score, int x, int y, int direction, float multiplier);
void chr_score_free(chr_score *score);
void chr_score_tick(chr_score *score);
void chr_score_render(chr_score *score);

// These are just quick plans, may change.
void chr_score_hit(chr_score *score, int points);
void chr_score_victory(chr_score *score, int health);
void chr_score_end_combo(chr_score *score, vec2i pos);
void chr_score_interrupt(chr_score *score, vec2i pos);

#endif // _SCORE_H
