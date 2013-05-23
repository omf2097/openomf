#ifndef _SCORE_H
#define _SCORE_H

#include "utils/list.h"
#include "video/texture.h"

enum {
    SCORE_EV_PUNCH = 0,
    SCORE_EV_KICK,
    SCORE_EV_ROUNDHOUSE,
    SCORE_EV_PERFECTROUND,
};

typedef struct chr_score_t {
    int score;
    int x,y;
    float multiplier;
    list texts;
    texture scoretex;
    
    int consecutive_hits;
    int combo_hits;
} chr_score;

void chr_score_create(chr_score *score, int x, int y, float multiplier);
void chr_score_free(chr_score *score);
void chr_score_tick(chr_score *score);
void chr_score_render(chr_score *score);

// Helper function for formatting the score
void chr_score_format(chr_score *score, char *buf);

// These are just quick plans, may change.
void chr_score_move(chr_score *score, int move);
void chr_score_victory(chr_score *score, int health);
void chr_score_interrupt(chr_score *score);

#endif // _SCORE_H