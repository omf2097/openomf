#include "game/score.h"
#include "video/surface.h"
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

void chr_score_create(chr_score *score, float multiplier) {
    score->multiplier = multiplier;
    score->x = 0;
    score->y = 0;
    score->direction = OBJECT_FACE_RIGHT;
    list_create(&score->texts);
    chr_score_reset(score);
}

void chr_score_reset(chr_score *score) {
    iterator it;
    score_text *t;

    score->score = 0;
    score->wins = 0;
    score->consecutive_hits = 0;
    score->consecutive_hit_score = 0;
    score->combo_hits = 0;
    score->combo_hit_score = 0;
    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        free(t->text);
        list_delete(&score->texts, &it);
    }
}

void chr_score_set_pos(chr_score *score, int x, int y, int direction) {
    score->x = x;
    score->y = y;
    score->direction = direction;
}

unsigned int chr_score_get_num_texts(chr_score *score) {
    return list_size(&score->texts);
}

void chr_score_free(chr_score *score) {
    iterator it;
    score_text *t;

    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        free(t->text);
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

void chr_score_format(unsigned int score, char *buf) {
    unsigned int n = 0;
    unsigned int scale = 1;
    while(score >= 1000) {
        n = n + scale * (score % 1000);
        score /= 1000;
        scale *= 1000;
    }
    int len = sprintf(buf, "%u", score);
    while(scale != 1) {
        scale /= 1000;
        score = n / scale;
        n = n  % scale;
        len += sprintf(buf + len, ",%03u", score);
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

void chr_score_add(chr_score *score, char *text, int points, vec2i pos, float position) {
    // Create texture
    // Add texture to list, set position to 1.0f, set points
    score_text s;
    s.text = text;
    s.points = points;
    s.start = pos;
    s.position = position;

    list_append(&score->texts, &s, sizeof(score_text));
}

void chr_score_hit(chr_score *score, int points) {
    score->score += points;
    score->consecutive_hits++;
    score->consecutive_hit_score += points;
    score->combo_hits++;
    score->combo_hit_score += points;
}

void chr_score_victory(chr_score *score, int health) {
    // Add texts for scrap bonus, perfect round, whatever
    score->wins++;
}

int chr_score_interrupt(chr_score *score, vec2i pos) {
    // Enemy interrupted somehow, show consecutive hits or whatevera
    int ret = 0;
    if (score->consecutive_hits > 3) {
        char *text = malloc(64);
        ret = 1;
        int len = sprintf(text, "%d consecutive hits ", score->consecutive_hits);
        chr_score_format(score->consecutive_hit_score, text+len);
        // XXX hardcode the y coordinate for now
        chr_score_add(score, text, score->consecutive_hit_score, vec2i_create(pos.x, 130), 1.0f);
    }
    score->consecutive_hits = 0;
    score->consecutive_hit_score = 0;
    return ret;
}

int chr_score_end_combo(chr_score *score, vec2i pos) {
    // enemy recovered control, end any combos
    int ret = 0;
    if (score->combo_hits > 1) {
        char *text = malloc(64);
        ret = 1;
        int len = sprintf(text, "%d hit combo ", score->combo_hits);
        chr_score_format(score->combo_hit_score*4, text+len);
        // XXX hardcode the y coordinate for now
        chr_score_add(score, text, score->combo_hit_score*4, vec2i_create(pos.x, 130), 1.0f);
    }
    score->combo_hits = 0;
    score->combo_hit_score = 0;
    return ret;
}

void chr_score_serialize(chr_score *score, serial *ser) {
    serial_write_int32(ser, score->score);
    serial_write_int8(ser, score->texts.size);
    iterator it;
    score_text *t;

    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        serial_write_int8(ser, strlen(t->text)+1);
        serial_write(ser, t->text, strlen(t->text)+1);
        serial_write_float(ser, t->position);
        serial_write_int16(ser, t->start.x);
        serial_write_int16(ser, t->start.y);
        serial_write_int32(ser, t->points);
    }
}

void chr_score_unserialize(chr_score *score, serial *ser) {
    score->score = serial_read_int32(ser);
    uint8_t count = serial_read_int8(ser);
    uint16_t text_len;
    char *text;
    float pos;
    int x, y;
    int points;

    // clean it out
    chr_score_free(score);
    list_create(&score->texts);

    for (int i = 0; i < count; i++) {
        text_len = serial_read_int8(ser);
        text = malloc(text_len);
        serial_read(ser, text, text_len);
        pos = serial_read_float(ser);
        x = serial_read_int16(ser);
        y = serial_read_int16(ser);
        points = serial_read_int32(ser);

        chr_score_add(score, text, points, vec2i_create(x, y), pos);
    }
}
