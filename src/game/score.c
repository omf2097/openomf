#include "game/score.h"
#include "video/texture.h"
#include <stdio.h>

typedef struct score_text_t {
    texture tex;
    float position; // Position of text between middle of screen and (x,y). 1.0 at middle, 0.0 at end
    int points;
} score_text;

int base_scores[] = {
    800,
    1200,
    2800,
    100000,
    100000,
    100000,
};

float multipliers[] = {
    0.4,
    0.8,
    1.2,
    1.6,
    2.0,
    3.0,
    4.0
};

void chr_score_create(chr_score *score, int x, int y, float multiplier) {
    score->score = 0;
    score->x = x;
    score->y = y;
    score->consecutive_hits = 0;
    score->combo_hits = 0;
    score->multiplier = multiplier;
    list_create(&score->texts);
}

void chr_score_free(chr_score *score) {
    // Free textures
    iterator it;
    score_text *t;
    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        texture_free(&t->tex);
    } 
    list_free(&score->texts);
}

void chr_score_tick(chr_score *score) {
    iterator it;
    score_text *t;
    
    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        t->position -= 0.01f;
        if(t->position < 0.0f) {
            score->score += t->points;
            texture_free(&t->tex);
            list_delete(&score->texts, &it);
        }
    } 
}

void chr_score_format(chr_score *score, char *buf) {
    int len;
    int millions = 0;
    int thousands = 0;
    if (score->score >= 100000000) {
        millions = score->score / 1000000;
        len = sprintf(buf, "%d,", millions);
        buf += len;
    }
    if (score->score >= 1000) {
        thousands = (score->score - (millions * 1000000)) / 1000;
        if (millions) {
            len = sprintf(buf, "%03d,", thousands);
        } else {
            len = sprintf(buf, "%d,", thousands);
        }
        buf += len;
    }
    if (millions || thousands) {
        sprintf(buf, "%03d",  (score->score - (millions * 1000000) - (thousands * 1000)));
    } else {
        sprintf(buf, "%d",  (score->score - (millions * 1000000) - (thousands * 1000)));
    }
}

void chr_score_render(chr_score *score) {
    // Render all texts in list to right spot
}

void chr_score_add(chr_score *score, const char *text, int points) {
    // Create texture
    // Add texture to list, set position to 1.0f, set points
}

void chr_score_move(chr_score *score, int move) {
    score->score += base_scores[move];
}

void chr_score_victory(chr_score *score, int health) {
    // Add texts for scrap bonus, perfect round, whatever
}

void chr_score_interrupt(chr_score *score) {
    // Enemy interrupted somehow, show consecutive hits or whatever
}
