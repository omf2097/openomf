#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "game/gui/textselector.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"

typedef struct {
    char *text;
    text_settings tconf;
    int ticks;
    int dir;
    int pos_;
    int *pos;
    vector options;

    void *userdata;
    textselector_toggle_cb toggle;
} textselector;

void textselector_clear_options(component *c) {
    textselector *tb = widget_get_obj(c);

    // Free all memory pointers
    iterator it;
    char **text;
    vector_iter_begin(&tb->options, &it);
    while((text = iter_next(&it)) != NULL) {
        omf_free(*text);
    }

    // Clear vector
    vector_clear(&tb->options);
}

void textselector_add_option(component *c, const char *value) {
    textselector *tb = widget_get_obj(c);
    char *new = strdup(value);
    vector_append(&tb->options, &new);
}

const char *textselector_get_current_text(const component *c) {
    textselector *tb = widget_get_obj(c);
    char **text = vector_get(&tb->options, *tb->pos);
    if(text != NULL) {
        return *text;
    }
    return NULL;
}

static void textselector_render(component *c) {
    textselector *tb = widget_get_obj(c);
    char buf[100];
    int buf_max = sizeof buf - 1;
    buf[buf_max] = '\0';

    if(vector_size(&tb->options) > 0 && tb->text[0] != '\0') {
        // label & options
        char **opt = vector_get(&tb->options, *tb->pos);
        snprintf(buf, buf_max, "%s %s", tb->text, *opt);
    } else if(vector_size(&tb->options) > 0) {
        // no label, just options
        char **opt = vector_get(&tb->options, *tb->pos);
        snprintf(buf, buf_max, "%s", *opt);
    } else {
        // no options, just label
        snprintf(buf, buf_max, "%s -", tb->text);
    }

    // Render text
    text_mode mode = TEXT_UNSELECTED;
    if(component_is_selected(c)) {
        mode = TEXT_SELECTED;
    } else if(component_is_disabled(c)) {
        mode = TEXT_DISABLED;
    }
    text_render(&tb->tconf, mode, c->x, c->y, c->w, c->h, buf);
}

static int textselector_action(component *c, int action) {
    textselector *tb = widget_get_obj(c);
    if(action == ACT_KICK || action == ACT_PUNCH || action == ACT_RIGHT) {
        if(vector_size(&tb->options) == 0) {
            return 0;
        }
        (*tb->pos)++;
        if(*tb->pos >= (int)vector_size(&tb->options)) {
            *tb->pos = 0;
        }
        if(tb->toggle) {
            tb->toggle(c, tb->userdata, *tb->pos);
        }
        audio_play_sound(20, 0.5f, 0.5f, 2.0f);
        // reset ticks so text is bright
        tb->ticks = 0;
        tb->dir = 0;
        return 0;
    } else if(action == ACT_LEFT) {
        if(vector_size(&tb->options) == 0) {
            return 0;
        }
        (*tb->pos)--;
        if(*tb->pos < 0) {
            *tb->pos = vector_size(&tb->options) - 1;
        }
        if(tb->toggle) {
            tb->toggle(c, tb->userdata, *tb->pos);
        }
        audio_play_sound(20, 0.5f, -0.5f, 2.0f);
        // reset ticks so text is bright
        tb->ticks = 0;
        tb->dir = 0;
        return 0;
    }
    return 1;
}

static void textselector_tick(component *c) {
    textselector *tb = widget_get_obj(c);
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

int textselector_get_pos(const component *c) {
    textselector *tb = widget_get_obj(c);
    return *tb->pos;
}

void textselector_set_pos(component *c, int pos) {
    textselector *tb = widget_get_obj(c);
    *tb->pos = pos;
}

static void textselector_free(component *c) {
    textselector *tb = widget_get_obj(c);
    textselector_clear_options(c);
    vector_free(&tb->options);
    omf_free(tb->text);
    omf_free(tb);
}

component *textselector_create(const text_settings *tconf, const char *text, const char *help,
                               textselector_toggle_cb cb, void *userdata) {
    component *c = widget_create();

    textselector *tb = omf_calloc(1, sizeof(textselector));
    component_set_help_text(c, help);
    tb->text = strdup(text);
    memcpy(&tb->tconf, tconf, sizeof(text_settings));
    tb->pos = &tb->pos_;
    tb->userdata = userdata;
    tb->toggle = cb;
    vector_create(&tb->options, sizeof(char *));
    widget_set_obj(c, tb);

    component_set_size_hints(c, text_width(&tb->tconf, text), 8);

    widget_set_render_cb(c, textselector_render);
    widget_set_action_cb(c, textselector_action);
    widget_set_tick_cb(c, textselector_tick);
    widget_set_free_cb(c, textselector_free);

    return c;
}

component *textselector_create_bind(const text_settings *tconf, const char *text, const char *help,
                                    textselector_toggle_cb toggle_cb, void *userdata, int *bind) {
    component *c = textselector_create(tconf, text, help, toggle_cb, userdata);
    textselector *ts = widget_get_obj(c);
    ts->pos = (bind) ? bind : &ts->pos_;
    return c;
}

component *textselector_create_bind_opts(const text_settings *tconf, const char *text, const char *help,
                                         textselector_toggle_cb toggle_cb, void *userdata, int *bind, const char **opts,
                                         int opt_size) {
    component *c = textselector_create_bind(tconf, text, help, toggle_cb, userdata, bind);
    for(int i = 0; i < opt_size; i++) {
        textselector_add_option(c, opts[i]);
    }
    return c;
}
