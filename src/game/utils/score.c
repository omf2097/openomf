#include "game/utils/score.h"
#include "game/utils/formatting.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "video/surface.h"
#include <math.h>
#include <stdio.h>

#define TEXT_HUD_COLOR 0xE7
#define TEXT_HUD_SHADOW 0xF8

#define SLIDER_DISTANCE 50
#define SLIDER_HANG_TIME 25

#define SCRAP 100000
#define DESTRUCTION 200000

typedef struct score_text {
    text *text;
    float position; // Position of text between middle of screen and (x,y). 1.0 at middle, 0.0 at end
    vec2i start;
    int points;
    int age;
} score_text;

float std_multipliers[] = {
    0.2f, // punching bag
    0.4f, // rookie
    0.6f, // veteran
    0.8f, // world class
    1.0f, // champion
    1.2f, // deadly
    1.4f  // ultimate
};

// Some of the difficulties do not map to tournament.
float tournament_multipliers[] = {
    0.0f, // unused
    0.0f, // unused
    0.5f, // aluminum - veteran
    0.0f, // unused
    1.0f, // iron - champion
    1.5f, // steel - deadly
    2.0f, // heavy metal - ultimate
};

vec2i interpolate(vec2i start, vec2i end, float fraction) {
    int nx = start.x + (end.x - start.x) * fraction;
    int ny = start.y + (end.y - start.y) * fraction;
    return vec2i_create(nx, ny);
}

static text *create_text_obj(const char *str) {
    text *obj = text_create_with_font_and_size(FONT_SMALL, 155, 6);
    text_set_color(obj, TEXT_HUD_COLOR);
    text_set_shadow_color(obj, TEXT_HUD_SHADOW);
    text_set_shadow_style(obj, GLYPH_SHADOW_RIGHT | GLYPH_SHADOW_BOTTOM);
    text_set_word_wrap(obj, false);
    text_set_from_c(obj, str);
    text_generate_layout(obj);
    return obj;
}

static void set_score_text(text *dst, int score) {
    char tmp[50];
    score_format(score, tmp, 50);
    text_set_from_c(dst, tmp);
}

void chr_score_create(chr_score *score) {
    score->difficulty = 0; // will be set later
    score->x = 0;
    score->y = 0;
    score->direction = OBJECT_FACE_RIGHT;
    score->multipliers = std_multipliers;
    score->total = create_text_obj("0");
    list_create(&score->texts);
    chr_score_reset(score, 1);
    chr_score_reset_wins(score);
}

void chr_score_set_difficulty(chr_score *score, int difficulty) {
    score->difficulty = difficulty;
}

void chr_score_set_tournament_mode(chr_score *score, bool tournament) {
    score->multipliers = tournament ? tournament_multipliers : std_multipliers;
}

void chr_score_reset(chr_score *score, bool wipe) {
    iterator it;
    score_text *t;

    if(wipe) {
        score->score = 0;
        text_set_from_c(score->total, "0");
    }
    score->rounds = 0;
    score->consecutive_hits = 0;
    score->consecutive_hit_score = 0;
    score->combo_hits = 0;
    score->combo_hit_score = 0;
    score->done = false;
    score->scrap = false;
    score->destruction = false;
    list_iter_begin(&score->texts, &it);
    foreach(it, t) {
        text_free(&t->text);
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
    if(score->direction == OBJECT_FACE_LEFT) {
        text_set_horizontal_align(score->total, TEXT_ALIGN_RIGHT);
    }
}

unsigned int chr_score_get_num_texts(chr_score *score) {
    return list_size(&score->texts);
}

int chr_score_onscreen(chr_score *score) {
    return list_size(&score->texts) > 0;
}

float chr_score_get_difficulty_multiplier(chr_score *score) {
    return score->multipliers[score->difficulty];
}

void chr_score_free(chr_score *score) {
    iterator it;
    score_text *t;

    list_iter_begin(&score->texts, &it);
    foreach(it, t) {
        text_free(&t->text);
    }
    list_free(&score->texts);

    text_free(&score->total);
}

void chr_score_tick(chr_score *score) {
    iterator it;
    score_text *t;
    bool text_needs_refresh = false;
    int lastage = -1;

    list_iter_begin(&score->texts, &it);
    foreach(it, t) {
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
            text_needs_refresh = true;
            text_free(&t->text);
            list_delete(&score->texts, &it);
        }
    }

    if(text_needs_refresh) {
        set_score_text(score->total, score->score);
    }
}

void chr_score_render(chr_score *score, bool render_total_points) {
    if(render_total_points) {
        // Correct x position by text object width
        int x_correction = score->direction == OBJECT_FACE_LEFT ? 155 : 0;
        text_draw(score->total, score->x - x_correction, score->y);
    }

    // Render all texts in list to right spot
    iterator it;
    score_text *t;
    int lastage = -1;
    vec2i pos;

    list_iter_begin(&score->texts, &it);
    foreach(it, t) {
        if(lastage > 0 && (lastage - t->age) < SLIDER_DISTANCE) {
            break;
        }
        if(score->direction == OBJECT_FACE_RIGHT) {
            pos = interpolate(vec2i_create(score->x, score->y), t->start, t->position);
        } else {
            pos = interpolate(vec2i_create(score->x - text_get_layout_width(t->text), score->y), t->start, t->position);
        }
        text_draw(t->text, pos.x, pos.y);
        lastage = t->age;
    }
}

void chr_score_add(chr_score *score, char *text, int points, vec2i pos, float position) {
    // Create texture
    // Add texture to list, set position to 1.0f, set points
    score_text s;
    s.text = create_text_obj(text);
    s.points = points;
    s.start = pos;
    // center correctly initially, but end up justified
    s.start.x -= text_get_layout_width(s.text) / 2;
    s.position = position;
    s.age = 0;

    list_append(&score->texts, &s, sizeof(score_text));
}

void chr_score_hit(chr_score *score, int points) {
    points = points * chr_score_get_difficulty_multiplier(score);
    score->score += points;
    score->consecutive_hits++;
    score->consecutive_hit_score += points;
    score->combo_hits++;
    score->combo_hit_score += points;
    set_score_text(score->total, score->score);
}

void chr_score_victory(chr_score *score, int health) {
    char tmp[64];
    // Add texts for scrap bonus, perfect round, whatever
    score->wins++;
    score->health = health;
    if(health == 100) {
        int len = snprintf(tmp, 64, "perfect round ");
        int points = DESTRUCTION * chr_score_get_difficulty_multiplier(score);
        score_format(points, tmp + len, 64 - len);
        // XXX hardcode the y coordinate for now
        chr_score_add(score, tmp, points, vec2i_create(160, 100), 1.0f);
    }

    int len = snprintf(tmp, 64, "vitality ");
    int points = truncf((DESTRUCTION * chr_score_get_difficulty_multiplier(score)) * (health / 100.0f));
    score_format(points, tmp + len, 64 - len);
    // XXX hardcode the y coordinate for now
    chr_score_add(score, tmp, points, vec2i_create(160, 100), 1.0f);
}

void chr_score_scrap(chr_score *score) {
    score->scrap = true;
}

void chr_score_destruction(chr_score *score) {
    score->destruction = true;
}

void chr_score_done(chr_score *score) {
    if(!score->done) {
        char text[64];
        score->done = true;
        if(score->destruction) {
            int len = snprintf(text, 64, "destruction bonus ");
            int points = DESTRUCTION * chr_score_get_difficulty_multiplier(score);
            score_format(points, text + len, 64 - len);
            // XXX hardcode the y coordinate for now
            chr_score_add(score, text, points, vec2i_create(160, 100), 1.0f);
            score->destruction = false;
        } else if(score->scrap) {
            int len = snprintf(text, 64, "scrap bonus ");
            int points = SCRAP * chr_score_get_difficulty_multiplier(score);
            score_format(points, text + len, 64 - len);
            // XXX hardcode the y coordinate for now
            chr_score_add(score, text, points, vec2i_create(160, 100), 1.0f);
            score->scrap = false;
        }
    }
}

void chr_score_clear_done(chr_score *score) {
    score->done = false;
}

int chr_score_interrupt(chr_score *score, vec2i pos) {
    // Enemy interrupted somehow, show consecutive hits or whatevera
    int ret = 0;
    char text[64];
    if(score->consecutive_hits > 3) {
        ret = 1;
        int len = snprintf(text, 64, "%d consecutive hits", score->consecutive_hits);
        if(score->consecutive_hit_score > 0) {
            text[len++] = ' ';
            score_format(score->consecutive_hit_score, text + len, 64 - len);
        }
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
    char text[64];
    if(score->combo_hits > 1) {
        ret = 1;
        int len = snprintf(text, 64, "%d hit combo", score->combo_hits);
        if(score->combo_hit_score > 0) {
            text[len++] = ' ';
            score_format(score->combo_hit_score * 4, text + len, 64 - len);
        }
        // XXX hardcode the y coordinate for now
        chr_score_add(score, text, score->combo_hit_score * 4, vec2i_create(pos.x, 130), 1.0f);
    }
    score->combo_hits = 0;
    score->combo_hit_score = 0;
    return ret;
}

int chr_score_clone(chr_score *src, chr_score *dst) {
    iterator it;
    score_text *t;
    memcpy(dst, src, sizeof(chr_score));
    dst->total = create_text_obj(text_c(src->total));
    text_set_horizontal_align(dst->total, text_get_horizontal_align(src->total));
    list_create(&dst->texts);
    list_iter_begin(&src->texts, &it);
    foreach(it, t) {
        score_text t2;
        memcpy(&t2, t, sizeof(score_text));
        t2.text = create_text_obj(text_c(t->text));
        text_set_horizontal_align(t2.text, text_get_horizontal_align(t->text));
        list_append(&dst->texts, &t2, sizeof(score_text));
    }
    return 0;
}
