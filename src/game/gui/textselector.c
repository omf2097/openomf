#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "game/gui/text/text.h"
#include "game/gui/textselector.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"

typedef struct text_selector {
    text *text;
    str title;
    int ticks;
    int dir;
    int pos_;
    int *pos;
    vector options;

    font_size override_font;
    text_horizontal_align text_horizontal_align;
    text_vertical_align text_vertical_align;

    void *userdata;
    textselector_toggle_cb toggle;
} text_selector;

void textselector_clear_options(component *c) {
    text_selector *t = widget_get_obj(c);
    vector_clear(&t->options);
}

void textselector_add_option(component *c, const char *value) {
    text_selector *tb = widget_get_obj(c);
    char *new = omf_strdup(value);
    vector_append(&tb->options, &new);
}

const char *textselector_get_current_text(const component *c) {
    text_selector *tb = widget_get_obj(c);
    char **text = vector_get(&tb->options, *tb->pos);
    if(text != NULL) {
        return *text;
    }
    return NULL;
}

static void refresh(component *c) {
    text_selector *t = widget_get_obj(c);
    str new;
    if(vector_size(&t->options) > 0 && str_size(&t->title) > 0) {
        // label & options
        char **opt = vector_get(&t->options, *t->pos);
        if(opt) {
            str_from_format(&new, "%s %s", str_c(&t->title), *opt);
        } else {
            str_from_format(&new, "%s NULL", str_c(&t->title));
        }
    } else if(vector_size(&t->options) > 0) {
        // no label, just options
        char **opt = vector_get(&t->options, *t->pos);
        str_from_format(&new, "%s", *opt);
    } else {
        // no options, just label
        str_from_format(&new, "%s -", str_c(&t->title));
    }
    text_set_from_str(t->text, &new);
    str_free(&new);
}

static void textselector_render(component *c) {
    text_selector *t = widget_get_obj(c);
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

static int textselector_action(component *c, int action) {
    text_selector *tb = widget_get_obj(c);
    if(vector_size(&tb->options) <= 1) {
        return 0;
    }
    int old_pos = *tb->pos;
    float panning = 0.0f;
    if(action == ACT_KICK || action == ACT_PUNCH || action == ACT_RIGHT) {
        panning = 0.5f;
        (*tb->pos)++;
        if(*tb->pos >= (int)vector_size(&tb->options)) {
            *tb->pos = 0;
        }
    } else if(action == ACT_LEFT) {
        panning = -0.5f;
        (*tb->pos)--;
        if(*tb->pos < 0) {
            *tb->pos = vector_size(&tb->options) - 1;
        }
    }
    if(old_pos != *tb->pos) {
        refresh(c);
        if(tb->toggle) {
            tb->toggle(c, tb->userdata, *tb->pos);
        }
        audio_play_sound(20, 0.5f, panning, 2.0f);
        // reset ticks so text is bright
        tb->ticks = 0;
        return 0;
    }
    return 1;
}

static void textselector_tick(component *c) {
    text_selector *t = widget_get_obj(c);
    t->ticks++;
}

int textselector_get_pos(const component *c) {
    text_selector *t = widget_get_obj(c);
    return *t->pos;
}

void textselector_set_pos(component *c, int pos) {
    text_selector *t = widget_get_obj(c);
    *t->pos = pos;
}

static void textselector_free(component *c) {
    text_selector *t = widget_get_obj(c);
    vector_free(&t->options);
    str_free(&t->title);
    text_free(&t->text);
    omf_free(t);
}

static void item_free(void *item) {
    omf_free(*(void **)item);
}

static void textselector_init(component *c, const gui_theme *theme) {
    text_selector *t = widget_get_obj(c);
    refresh(c);
    text_set_font(t->text, t->override_font != FONT_NONE ? t->override_font : theme->text.font);
    text_set_line_spacing(t->text, 0);
    if(c->w_hint < 0 && c->h_hint < 0) {
        text_generate_layout(t->text);
        int text_width = text_get_layout_width(t->text);
        int text_height = text_get_layout_height(t->text);
        component_set_size_hints(c, text_width, text_height);
    }
}

static void textselector_layout(component *c, int x, int y, int w, int h) {
    text_selector *t = widget_get_obj(c);
    text_set_bounding_box(t->text, w, h);
    text_set_horizontal_align(t->text, t->text_horizontal_align);
    text_set_vertical_align(t->text, t->text_vertical_align);
    text_generate_layout(t->text);
}

component *textselector_create(const char *text, const char *help, textselector_toggle_cb cb, void *userdata) {
    component *c = widget_create();

    text_selector *t = omf_calloc(1, sizeof(text_selector));
    str_from_c(&t->title, text);
    t->text = text_create();
    t->pos = &t->pos_;
    t->userdata = userdata;
    t->toggle = cb;
    t->override_font = FONT_NONE;
    t->text_vertical_align = TEXT_ALIGN_MIDDLE;
    t->text_horizontal_align = TEXT_ALIGN_CENTER;
    vector_create_with_size_cb(&t->options, sizeof(char *), 0, item_free);
    widget_set_obj(c, t);

    component_set_help_text(c, help);

    widget_set_render_cb(c, textselector_render);
    widget_set_action_cb(c, textselector_action);
    widget_set_tick_cb(c, textselector_tick);
    widget_set_free_cb(c, textselector_free);
    widget_set_init_cb(c, textselector_init);
    widget_set_layout_cb(c, textselector_layout);

    return c;
}

component *textselector_create_bind(const char *text, const char *help, textselector_toggle_cb toggle_cb,
                                    void *userdata, int *bind) {
    component *c = textselector_create(text, help, toggle_cb, userdata);
    text_selector *ts = widget_get_obj(c);
    ts->pos = (bind) ? bind : &ts->pos_;
    return c;
}

component *textselector_create_bind_opts(const char *text, const char *help, textselector_toggle_cb toggle_cb,
                                         void *userdata, int *bind, const char **opts, int opt_size) {
    component *c = textselector_create_bind(text, help, toggle_cb, userdata, bind);
    for(int i = 0; i < opt_size; i++) {
        textselector_add_option(c, opts[i]);
    }
    return c;
}

void textselector_set_font(component *c, font_size font) {
    text_selector *t = widget_get_obj(c);
    t->override_font = font;
}

void textselector_set_text_horizontal_align(component *c, text_horizontal_align align) {
    text_selector *t = widget_get_obj(c);
    t->text_horizontal_align = align;
}

void textselector_set_text_vertical_align(component *c, text_vertical_align align) {
    text_selector *t = widget_get_obj(c);
    t->text_vertical_align = align;
}
