#ifndef SCORE_H
#define SCORE_H

#include "game/gui/text/text.h"
#include "game/protos/object.h"
#include "utils/list.h"
#include "video/surface.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum
{
    SCORE_EV_PUNCH = 0,
    SCORE_EV_KICK,
    SCORE_EV_ROUNDHOUSE,
    SCORE_EV_PERFECTROUND,
};

typedef struct chr_score_t {
    int score;
    int rounds;
    int wins; // for 2 player
    int health;
    int x, y;
    int direction;
    int difficulty;
    list texts;
    text *total;

    int consecutive_hits;
    int consecutive_hit_score;
    int combo_hits;
    int combo_hit_score;
    float *multipliers;
    bool done;
    bool scrap;
    bool destruction;
} chr_score;

void chr_score_create(chr_score *score);
void chr_score_set_difficulty(chr_score *score, int difficulty);
void chr_score_set_tournament_mode(chr_score *score, bool tournament);
void chr_score_reset(chr_score *score, bool wipe);
void chr_score_reset_wins(chr_score *score);
void chr_score_set_pos(chr_score *score, int x, int y, int direction);
unsigned int chr_score_get_num_texts(chr_score *score);
void chr_score_free(chr_score *score);
void chr_score_tick(chr_score *score);
void chr_score_render(chr_score *score, bool render_total_points);
int chr_score_onscreen(chr_score *score);
float chr_score_get_difficulty_multiplier(chr_score *score);

void chr_score_hit(chr_score *score, int points);
void chr_score_victory(chr_score *score, int health);
void chr_score_scrap(chr_score *score);
void chr_score_destruction(chr_score *score);
void chr_score_done(chr_score *score);
void chr_score_clear_done(chr_score *score);
int chr_score_end_combo(chr_score *score, vec2i pos);
int chr_score_interrupt(chr_score *score, vec2i pos);

int chr_score_clone(chr_score *src, chr_score *dst);

#endif // SCORE_H
