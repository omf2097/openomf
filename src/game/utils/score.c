#include "game/utils/score.h"
#include "game/utils/formatting.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/surface.h"
#include <math.h>
#include <stdio.h>

#define TEXT_COLOR color_create(186, 250, 250, 255)
#define SLIDER_DISTANCE 50
#define SLIDER_HANG_TIME 25

#define SCRAP 100000
#define DESTRUCTION 200000

typedef struct score_text_t {
    char *text;
    float position; // Position of text between middle of screen and (x,y). 1.0 at middle, 0.0 at end
    vec2i start;
    int points;
    int age;
} score_text;

float multipliers[] = {
    0.2, // punching bag
    0.4, // rookie
    0.6, // veteran
    0.8, // world class
    1.0, // champion
    1.2, // deadly
    1.4  // ultimate
};

vec2i interpolate(vec2i start, vec2i end, float fraction) {
    int nx = start.x + (end.x - start.x) * fraction;
    int ny = start.y + (end.y - start.y) * fraction;
    return vec2i_create(nx, ny);
}

void chr_score_create(chr_score *score) {
    score->difficulty = 0; // will be set later
    score->x = 0;
    score->y = 0;
    score->direction = OBJECT_FACE_RIGHT;
    list_create(&score->texts);
    chr_score_reset(score, 1);
    chr_score_reset_wins(score);
}

void chr_score_set_difficulty(chr_score *score, int difficulty) {
    score->difficulty = difficulty;
}

void chr_score_reset(chr_score *score, int wipe) {
    iterator it;
    score_text *t;

    if(wipe) {
        score->score = 0;
    }
    score->rounds = 0;
    score->done = 0;
    score->consecutive_hits = 0;
    score->consecutive_hit_score = 0;
    score->combo_hits = 0;
    score->combo_hit_score = 0;
    score->scrap = 0;
    score->destruction = 0;
    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        omf_free(t->text);
        list_delete(&score->texts, &it);
    }
}

void chr_score_reset_wins(chr_score *score) {
    score->wins = 0;
}

void chr_score_set_pos(chr_score *score, int x, int y, int direction) {
    score->x = x;
    score->y = y;
    score->direction = direction;
}

unsigned int chr_score_get_num_texts(chr_score *score) {
    return list_size(&score->texts);
}

int chr_score_onscreen(chr_score *score) {
    return list_size(&score->texts) > 0;
}

void chr_score_free(chr_score *score) {
    iterator it;
    score_text *t;

    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        omf_free(t->text);
    }
    list_free(&score->texts);
}

void chr_score_tick(chr_score *score) {
    iterator it;
    score_text *t;
    int lastage = -1;

    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        // don't allow them to get too close together, if a bunch are added at once
        if(lastage > 0 && (lastage - t->age) < SLIDER_DISTANCE) {
            break;
        }
        if(t->age > SLIDER_HANG_TIME) {
            t->position -= 0.01f;
        }
        lastage = t->age++;
        if(t->position < 0.0f) {
            score->score += t->points;
            omf_free(t->text);
            list_delete(&score->texts, &it);
        }
    }
}

void chr_score_render(chr_score *score) {
    // Render all texts in list to right spot
    char tmp[50];
    score_format(score->score, tmp, 50);
    if(score->direction == OBJECT_FACE_RIGHT) {
        font_render_shadowed(&font_small, tmp, score->x, score->y, TEXT_COLOR, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM);
    } else {
        int s2len = strlen(tmp) * font_small.w;
        font_render_shadowed(&font_small, tmp, score->x - s2len, score->y, TEXT_COLOR,
                             TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM);
    }

    iterator it;
    score_text *t;
    int lastage = -1;
    vec2i pos;

    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        if(lastage > 0 && (lastage - t->age) < SLIDER_DISTANCE) {
            break;
        }
        pos = interpolate(vec2i_create(score->x, score->y), t->start, t->position);
        if(score->direction == OBJECT_FACE_LEFT) {
            pos =
                interpolate(vec2i_create(score->x - (strlen(t->text) * font_small.w), score->y), t->start, t->position);
        }
        font_render_shadowed(&font_small, t->text, pos.x, pos.y, TEXT_COLOR, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM);
        lastage = t->age;
    }
}

void chr_score_add(chr_score *score, char *text, int points, vec2i pos, float position) {
    // Create texture
    // Add texture to list, set position to 1.0f, set points
    score_text s;
    s.text = text;
    s.points = points;
    s.start = pos;
    // center correctly initially, but end up justified
    s.start.x -= ((strlen(s.text) * font_small.w) / 2);
    s.position = position;
    s.age = 0;

    list_append(&score->texts, &s, sizeof(score_text));
}

void chr_score_hit(chr_score *score, int points) {
    points = points * multipliers[score->difficulty];
    score->score += points;
    score->consecutive_hits++;
    score->consecutive_hit_score += points;
    score->combo_hits++;
    score->combo_hit_score += points;
}

void chr_score_victory(chr_score *score, int health) {
    // Add texts for scrap bonus, perfect round, whatever
    score->wins++;
    score->health = health;
    char *text;
    if(health == 100) {
        text = omf_calloc(64, 1);
        int len = snprintf(text, 64, "perfect round ");
        int points = DESTRUCTION * multipliers[score->difficulty];
        score_format(points, text + len, 64 - len);
        // XXX hardcode the y coordinate for now
        chr_score_add(score, text, points, vec2i_create(160, 100), 1.0f);
    }
    text = omf_calloc(64, 1);

    int len = snprintf(text, 64, "vitality ");
    int points = truncf((DESTRUCTION * multipliers[score->difficulty]) * (health / 100.0f));
    score_format(points, text + len, 64 - len);
    // XXX hardcode the y coordinate for now
    chr_score_add(score, text, points * (health / 100), vec2i_create(160, 100), 1.0f);
}

void chr_score_scrap(chr_score *score) {
    score->scrap = 1;
}

void chr_score_destruction(chr_score *score) {
    score->destruction = 1;
}

void chr_score_done(chr_score *score) {
    if(!score->done) {
        score->done = 1;
        if(score->destruction) {
            char *text = omf_calloc(64, 1);
            int len = snprintf(text, 64, "destruction bonus ");
            int points = DESTRUCTION * multipliers[score->difficulty];
            score_format(points, text + len, 64 - len);
            // XXX hardcode the y coordinate for now
            chr_score_add(score, text, points, vec2i_create(160, 100), 1.0f);
            score->destruction = 0;
        } else if(score->scrap) {
            char *text = omf_calloc(64, 1);
            int len = snprintf(text, 64, "scrap bonus ");
            int points = SCRAP * multipliers[score->difficulty];
            score_format(points, text + len, 64 - len);
            // XXX hardcode the y coordinate for now
            chr_score_add(score, text, points, vec2i_create(160, 100), 1.0f);
            score->scrap = 0;
        }
    }
}

void chr_score_clear_done(chr_score *score) {
    score->done = 0;
}

int chr_score_interrupt(chr_score *score, vec2i pos) {
    // Enemy interrupted somehow, show consecutive hits or whatevera
    int ret = 0;
    if(score->consecutive_hits > 3) {
        char *text = omf_calloc(64, 1);
        ret = 1;
        int len = snprintf(text, 64, "%d consecutive hits ", score->consecutive_hits);
        score_format(score->consecutive_hit_score, text + len, 64 - len);
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
    if(score->combo_hits > 1) {
        char *text = omf_calloc(64, 1);
        ret = 1;
        int len = snprintf(text, 64, "%d hit combo ", score->combo_hits);
        score_format(score->combo_hit_score * 4, text + len, 64 - len);
        // XXX hardcode the y coordinate for now
        chr_score_add(score, text, score->combo_hit_score * 4, vec2i_create(pos.x, 130), 1.0f);
    }
    score->combo_hits = 0;
    score->combo_hit_score = 0;
    return ret;
}

void chr_score_serialize(chr_score *score, serial *ser) {
    serial_write_int32(ser, score->score);
    serial_write_int32(ser, score->done);
    serial_write_int32(ser, score->scrap);
    serial_write_int32(ser, score->destruction);
    serial_write_int8(ser, score->texts.size);
    iterator it;
    score_text *t;

    list_iter_begin(&score->texts, &it);
    while((t = iter_next(&it)) != NULL) {
        serial_write_int8(ser, strlen(t->text) + 1);
        serial_write(ser, t->text, strlen(t->text) + 1);
        serial_write_float(ser, t->position);
        serial_write_int16(ser, t->start.x);
        serial_write_int16(ser, t->start.y);
        serial_write_int32(ser, t->points);
    }
}

void chr_score_unserialize(chr_score *score, serial *ser) {
    score->score = serial_read_int32(ser);
    score->done = serial_read_int32(ser);
    score->scrap = serial_read_int32(ser);
    score->destruction = serial_read_int32(ser);
    uint8_t count = serial_read_int8(ser);
    uint16_t text_len;
    char *text;
    float pos;
    int x, y;
    int points;

    // clean it out
    chr_score_free(score);
    list_create(&score->texts);

    for(int i = 0; i < count; i++) {
        text_len = serial_read_int8(ser);
        text = omf_calloc(text_len, 1);
        serial_read(ser, text, text_len);
        pos = serial_read_float(ser);
        x = serial_read_int16(ser);
        y = serial_read_int16(ser);
        points = serial_read_int32(ser);

        chr_score_add(score, text, points, vec2i_create(x, y), pos);
    }
}
