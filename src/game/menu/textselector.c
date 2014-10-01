#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "game/menu/textselector.h"
#include "game/menu/widget.h"
#include "audio/sound.h"
#include "utils/log.h"
#include "utils/compat.h"

typedef struct {
    char *text;
    const font *font;
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
        free(*text);
    }

    // Clear vector
    vector_clear(&tb->options);
}

void textselector_add_option(component *c, const char *value) {
    textselector *tb = widget_get_obj(c);
    char *new = strdup(value);
    vector_append(&tb->options, &new);
}

const char* textselector_get_current_text(const component *c) {
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
    int chars;
    int width;
    int xoff;

    // Only render if the selector has options
    if(vector_size(&tb->options) > 0) {
        char **opt = vector_get(&tb->options, *tb->pos);
        sprintf(buf, "%s %s", tb->text, *opt);
    } else {
        sprintf(buf, "%s -", tb->text);
    }

    // Render text
    chars = strlen(buf);
    width = chars*tb->font->w;
    xoff = (c->w - width)/2;
    if(component_is_selected(c)) {
        int t = tb->ticks / 2;
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(80 - t, 220 - t*2, 80 - t, 255));
    } else if (component_is_disabled(c)) {
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(121, 121, 121, 255));
    } else {
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(0, 121, 0, 255));
    }
}

static int textselector_action(component *c, int action) {
    textselector *tb = widget_get_obj(c);
    if(action == ACT_KICK || action == ACT_PUNCH || action == ACT_RIGHT) {
        if(vector_size(&tb->options) == 0) { return 0; }
        (*tb->pos)++;
        if (*tb->pos >= vector_size(&tb->options)) {
            *tb->pos = 0;
        }
        if(tb->toggle) {
            tb->toggle(c, tb->userdata, *tb->pos);
        }
        sound_play(20, 0.5f, 0.5f, 2.0f);
        return 0;
    } else  if(action == ACT_LEFT) {
        if(vector_size(&tb->options) == 0) { return 0; }
        (*tb->pos)--;
        if (*tb->pos < 0) {
            *tb->pos = vector_size(&tb->options) -1;
        }
        if(tb->toggle) {
            tb->toggle(c, tb->userdata, *tb->pos);
        }
        sound_play(20, 0.5f, -0.5f, 2.0f);
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
    free(tb->text);
    free(tb);
}

component* textselector_create(const font *font, const char *text, textselector_toggle_cb cb, void *userdata) {
    component *c = widget_create();

    textselector *tb = malloc(sizeof(textselector));
    memset(tb, 0, sizeof(textselector));
    tb->text = strdup(text);
    tb->font = font;
    tb->pos = &tb->pos_;
    tb->userdata = userdata;
    tb->toggle = cb;
    vector_create(&tb->options, sizeof(char*));
    widget_set_obj(c, tb);

    widget_set_render_cb(c, textselector_render);
    widget_set_action_cb(c, textselector_action);
    widget_set_tick_cb(c, textselector_tick);
    widget_set_free_cb(c, textselector_free);

    return c;
}

component* textselector_create_bind(const font *font, const char *text, textselector_toggle_cb toggle_cb, void *userdata, int *bind) {
    component* c = textselector_create(font, text, toggle_cb, userdata);
    textselector *ts = widget_get_obj(c);
    ts->pos = (bind) ? bind : &ts->pos_;
    return c;
}

component* textselector_create_bind_opts(const font *font, const char *text, textselector_toggle_cb toggle_cb, void *userdata, int *bind, const char **opts, int opt_size) {
    component* c = textselector_create_bind(font, text, toggle_cb, userdata, bind);
    for(int i = 0; i < opt_size; i++) {
        textselector_add_option(c, opts[i]);
    }
    return c;
}