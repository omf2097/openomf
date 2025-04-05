#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "game/gui/text/text.h"
#include "game/gui/textslider.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/str.h"

typedef struct text_slider {
    text *text;
    str title;
    int ticks;
    int pos_;
    int *pos;
    int has_off;
    int positions;
    bool disable_panning;

    font_size override_font;
    text_horizontal_align text_horizontal_align;
    text_vertical_align text_vertical_align;

    void *userdata;
    textslider_slide_cb slide;
} text_slider;

static void refresh(component *c) {
    text_slider *t = widget_get_obj(c);
    str txt;
    str_from_format(&txt, "%s ", str_c(&t->title));
    if(t->has_off && *t->pos == 0) {
        str_append_c(&txt, "OFF");
    } else {
        for(int i = 0; i < t->positions; i++) {
            if(i + 1 > *t->pos) {
                str_append_char(&txt, '|');
            } else {
                str_append_char(&txt, CURSOR_CHAR);
            }
        }
    }
    text_set_from_str(t->text, &txt);
    str_free(&txt);
}

static void textslider_render(component *c) {
    text_slider *t = widget_get_obj(c);
    const gui_theme *theme = component_get_theme(c);
    if(component_is_selected(c)) {
        text_set_color(t->text, theme->text.active_color);
    } else if(component_is_disabled(c)) {
        text_set_color(t->text, theme->text.disabled_color);
    } else {
        text_set_color(t->text, theme->text.inactive_color);
    }
    text_draw(t->text, c->x, c->y);
}

static int textslider_action(component *c, int action) {
    text_slider *t = widget_get_obj(c);
    int old_pos = *t->pos;
    float panning = t->disable_panning ? 0.0f : 0.5f;
    if(action == ACT_KICK || action == ACT_PUNCH || action == ACT_RIGHT) {
        (*t->pos)++;
        if(*t->pos > t->positions) {
            *t->pos = t->positions;
        }
    } else if(action == ACT_LEFT) {
        panning = -panning;
        (*t->pos)--;
        if(*t->pos < 0) {
            *t->pos = 0;
        }
    }
    if(old_pos != *t->pos) {
        // Play menu sound
        refresh(c);
        audio_play_sound(20, 0.5f, panning, 2.0f);
        if(t->slide) {
            t->slide(c, t->userdata, *t->pos);
        }
        // reset ticks so text is bright
        t->ticks = 0;
        return 0;
    }
    return 1;
}

static void textslider_tick(component *c) {
    text_slider *t = widget_get_obj(c);
    t->ticks++;
}

static void textslider_free(component *c) {
    text_slider *t = widget_get_obj(c);
    text_free(&t->text);
    str_free(&t->title);
    omf_free(t);
}

static void textslider_init(component *c, const gui_theme *theme) {
    text_slider *t = widget_get_obj(c);
    refresh(c);
    text_set_font(t->text, t->override_font != FONT_NONE ? t->override_font : theme->text.font);
    if(c->w_hint < 0 && c->h_hint < 0) {
        text_generate_layout(t->text);
        int text_width = text_get_layout_width(t->text);
        int text_height = text_get_layout_height(t->text);
        component_set_size_hints(c, text_width, text_height);
    }
}

static void textslider_layout(component *c, int x, int y, int w, int h) {
    text_slider *t = widget_get_obj(c);
    text_set_bounding_box(t->text, w, h);
    text_set_horizontal_align(t->text, t->text_horizontal_align);
    text_set_vertical_align(t->text, t->text_vertical_align);
    text_generate_layout(t->text);
}

component *textslider_create(const char *text, const char *help, unsigned int positions, int has_off,
                             textslider_slide_cb cb, void *userdata) {
    component *c = widget_create();

    text_slider *t = omf_calloc(1, sizeof(text_slider));
    t->text = text_create();
    t->ticks = 0;
    t->pos_ = 1;
    t->pos = &t->pos_;
    t->has_off = has_off;
    t->positions = positions;
    t->userdata = userdata;
    t->slide = cb;
    t->override_font = FONT_NONE;
    t->text_vertical_align = TEXT_ALIGN_MIDDLE;
    t->text_horizontal_align = TEXT_ALIGN_CENTER;
    str_from_c(&t->title, text);
    widget_set_obj(c, t);

    component_set_help_text(c, help);

    widget_set_render_cb(c, textslider_render);
    widget_set_action_cb(c, textslider_action);
    widget_set_tick_cb(c, textslider_tick);
    widget_set_free_cb(c, textslider_free);
    widget_set_init_cb(c, textslider_init);
    widget_set_layout_cb(c, textslider_layout);
    return c;
}

component *textslider_create_bind(const char *text, const char *help, unsigned int positions, int has_off,
                                  textslider_slide_cb cb, void *userdata, int *bind) {
    component *c = textslider_create(text, help, positions, has_off, cb, userdata);
    text_slider *ts = widget_get_obj(c);
    ts->pos = (bind) ? bind : &ts->pos_;
    return c;
}

void textslider_disable_panning(component *c) {
    text_slider *t = widget_get_obj(c);
    t->disable_panning = true;
}

void textslider_set_font(component *c, font_size font) {
    text_slider *t = widget_get_obj(c);
    t->override_font = font;
}

void textslider_set_text_horizontal_align(component *c, text_horizontal_align align) {
    text_slider *t = widget_get_obj(c);
    t->text_horizontal_align = align;
}

void textslider_set_text_vertical_align(component *c, text_vertical_align align) {
    text_slider *t = widget_get_obj(c);
    t->text_vertical_align = align;
}
