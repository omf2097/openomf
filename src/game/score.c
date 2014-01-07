#include "game/score.h"
#include "video/texture.h"
#include <stdio.h>

#define TEXT_COLOR color_create(186,250,250,255)

typedef struct score_text_t {
    char *text;
    float position; // Position of text between middle of screen and (x,y). 1.0 at middle, 0.0 at end
    vec2i start;
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

vec2i interpolate(vec2i start, vec2i end, float fraction) {
    int nx = start.x+(end.x - start.x)*fraction;
    int ny = start.y+(end.y - start.y)*fraction;
    return vec2i_create(nx, ny);
}

void chr_score_create(chr_score *score, int x, int y,  int direction, float multiplier) {
    score->score = 0;
    score->x = x;
    score->y = y;
    score->direction = direction;
    score->consecutive_hits = 0;
    score->consecutive_hit_score = 0;
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
        free(&t->text);
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
            free(t->text);
            list_delete(&score->texts, &it);
        }
    } 
}

void chr_score_format(int score, char *buf) {
    int len;
    int millions = 0;
    int thousands = 0;
    if (score >= 100000000) {
        millions = score / 1000000;
        len = sprintf(buf, "%d,", millions);
        buf += len;
    }
    if (score >= 1000) {
        thousands = (score - (millions * 1000000)) / 1000;
        if (millions) {
            len = sprintf(buf, "%03d,", thousands);
        } else {
            len = sprintf(buf, "%d,", thousands);
        }
        buf += len;
    }
    if (millions || thousands) {
        sprintf(buf, "%03d",  (score - (millions * 1000000) - (thousands * 1000)));
    } else {
        sprintf(buf, "%d",  (score - (millions * 1000000) - (thousands * 1000)));
    }
}

void chr_score_render(chr_score *score) {
    // Render all texts in list to right spot
    char tmp[50];
    chr_score_format(score->score, tmp);
    if (score->direction == OBJECT_FACE_RIGHT) {
        font_render(&font_small, tmp, score->x, score->y, TEXT_COLOR);
    } else {
        int s2len = strlen(tmp) * font_small.w;
        font_render(&font_small, tmp, score->x-s2len, score->y, TEXT_COLOR);
    }

    iterator it;
    score_text *t;

    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        vec2i pos = interpolate(vec2i_create(score->x, score->y), t->start, t->position);
        if (score->direction == OBJECT_FACE_LEFT) {
            pos = interpolate(vec2i_create(score->x-64, score->y), t->start, t->position);
        }
        font_render(&font_small, t->text, pos.x-(strlen(t->text)/2), pos.y, TEXT_COLOR);
    }
}

void chr_score_add(chr_score *score, char *text, int points, vec2i pos) {
    // Create texture
    // Add texture to list, set position to 1.0f, set points
    score_text s;
    s.text = text;
    s.points = points;
    s.start = pos;
    s.position = 1.0f;

    list_append(&score->texts, &s, sizeof(score_text));
}

void chr_score_hit(chr_score *score, int points) {
    score->score += points;
    score->consecutive_hits++;
    score->consecutive_hit_score += points;
}

void chr_score_victory(chr_score *score, int health) {
    // Add texts for scrap bonus, perfect round, whatever
}

void chr_score_interrupt(chr_score *score, vec2i pos) {
    // Enemy interrupted somehow, show consecutive hits or whatevera
    char *text = malloc(64);
    if (score->consecutive_hits > 3) {
        int len = sprintf(text, "%d consecutive hits ", score->consecutive_hits);
        chr_score_format(score->consecutive_hit_score, text+len);
        // XXX hardcode the y coordinate for now
        chr_score_add(score, text, score->consecutive_hit_score, vec2i_create(pos.x, 130));
    }
    score->consecutive_hits = 0;
    score->consecutive_hit_score = 0;
}
