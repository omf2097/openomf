#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "game/gui/textslider.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/str.h"

typedef struct {
    char *text;
    text_settings tconf;
    int ticks;
    int dir;
    int pos_;
    int *pos;
    int has_off;
    int positions;

    void *userdata;
    textslider_slide_cb slide;
} textslider;

static void textslider_render(component *c) {
    textslider *tb = widget_get_obj(c);
    str txt;
    str_from_format(&txt, "%s ", tb->text);
    if(tb->has_off && *tb->pos == 0) {
        str_append_c(&txt, "OFF");
    } else {
        for(int i = 0; i < tb->positions; i++) {
            if(i + 1 > *tb->pos) {
                str_append_c(&txt, "|");
            } else {
                str_append_c(&txt, "\x7f");
            }
        }
    }

    text_mode mode = TEXT_UNSELECTED;
    if(component_is_selected(c)) {
        mode = TEXT_SELECTED;
    } else if(component_is_disabled(c)) {
        mode = TEXT_DISABLED;
    }
    text_render_str(&tb->tconf, mode, c->x, c->y, c->w, c->h, &txt);
    str_free(&txt);
}

static int textslider_action(component *c, int action) {
    textslider *tb = widget_get_obj(c);
    int old_pos = *tb->pos;
    float panning = 0.5f;
    if(action == ACT_KICK || action == ACT_PUNCH || action == ACT_RIGHT) {
        (*tb->pos)++;
        if(*tb->pos > tb->positions) {
            *tb->pos = tb->positions;
        }
    } else if(action == ACT_LEFT) {
        panning = -panning;
        (*tb->pos)--;
        if(*tb->pos < 0) {
            *tb->pos = 0;
        }
    }
    if(old_pos != *tb->pos) {
        // Play menu sound
        audio_play_sound(20, 0.5f, panning, 2.0f);
        if(tb->slide) {
            tb->slide(c, tb->userdata, *tb->pos);
        }
        // reset ticks so text is bright
        tb->ticks = 0;
        tb->dir = 0;
        return 0;
    }
    return 1;
}

static void textslider_tick(component *c) {
    textslider *tb = widget_get_obj(c);
    if(!tb->dir) {
        tb->ticks++;
    } else {
        tb->ticks--;
    }
    if(tb->ticks > 120) {
        tb->dir = 1;
    }
    if(tb->ticks == 0) {
        tb->dir = 0;
    }
}

static void textslider_free(component *c) {
    textslider *tb = widget_get_obj(c);
    omf_free(tb->text);
    omf_free(tb);
}

component *textslider_create(const text_settings *tconf, const char *text, const char *help, unsigned int positions,
                             int has_off, textslider_slide_cb cb, void *userdata) {
    component *c = widget_create();

    textslider *tb = omf_calloc(1, sizeof(textslider));
    tb->text = omf_strdup(text);
    memcpy(&tb->tconf, tconf, sizeof(text_settings));
    component_set_help_text(c, help);
    tb->ticks = 0;
    tb->dir = 0;
    tb->pos_ = 1;
    tb->pos = &tb->pos_;
    tb->has_off = has_off;
    tb->positions = positions;
    tb->userdata = userdata;
    tb->slide = cb;
    widget_set_obj(c, tb);

    component_set_size_hints(c, text_width(&tb->tconf, text), 8);

    widget_set_render_cb(c, textslider_render);
    widget_set_action_cb(c, textslider_action);
    widget_set_tick_cb(c, textslider_tick);
    widget_set_free_cb(c, textslider_free);
    return c;
}

component *textslider_create_bind(const text_settings *tconf, const char *text, const char *help,
                                  unsigned int positions, int has_off, textslider_slide_cb cb, void *userdata,
                                  int *bind) {
    component *c = textslider_create(tconf, text, help, positions, has_off, cb, userdata);
    textslider *ts = widget_get_obj(c);
    ts->pos = (bind) ? bind : &ts->pos_;
    return c;
}
