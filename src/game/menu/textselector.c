#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "game/menu/textselector.h"
#include "audio/sound.h"
#include "utils/log.h"
#include "utils/compat.h"

typedef struct {
    char *text;
    font *font;
    int ticks;
    int dir;
    int pos_;
    int *pos;
    vector options;
} textselector;

void textselector_create(component *c, font *font, const char *text, const char *initialvalue) {
    component_create(c);

    textselector *tb = malloc(sizeof(textselector));
    tb->text = strdup(text);
    tb->font = font;
    tb->ticks = 0;
    tb->dir = 0;
    tb->pos_ = 0;
    tb->pos = &tb->pos_;
    vector_create(&tb->options, sizeof(char*));
    vector_append(&tb->options, &initialvalue);
    component_set_obj(c, tb);

    component_set_render_cb(c, textselector_render);
    component_set_action_cb(c, textselector_action);
    component_set_tick_cb(c, textselector_tick);
}

void textselector_clear_options(component *c) {
    textselector *tb = c->obj;
    vector_clear(&tb->options);
}

void textselector_add_option(component *c, const char *value) {
    textselector *tb = c->obj;
    vector_append(&tb->options, &value);
}

void textselector_free(component *c) {
    textselector *tb = c->obj;
    vector_free(&tb->options);
    free(tb->text);
    free(tb);
    component_free(c);
}

const char* textselector_get_current_text(component *c) {
    textselector *tb = c->obj;
    return (char*)(*(void**)vector_get(&tb->options, *tb->pos));
}

void textselector_render(component *c) {
    textselector *tb = c->obj;
    char buf[100];
    int chars;
    int width;
    int xoff;
    char **opt = vector_get(&tb->options, *tb->pos);
    sprintf(buf, "%s %s", tb->text, *opt);
    chars = strlen(buf);
    width = chars*tb->font->w;
    xoff = (c->w - width)/2;
    if(c->selected) {
        int t = tb->ticks / 2;
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(80 - t, 220 - t*2, 80 - t, 255));
    } else if (c->disabled) {
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(121, 121, 121, 255));
    } else {
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(0, 121, 0, 255));
    }
}

int textselector_action(component *c, int action) {
    textselector *tb = c->obj;
    if (action == ACT_KICK || action == ACT_PUNCH || action == ACT_RIGHT) {
        (*tb->pos)++;
        if (*tb->pos >= vector_size(&tb->options)) {
            *tb->pos = 0;
        }
        component_toggle(c, *tb->pos);
        sound_play(20, 0.5f, 0.5f, 2.0f);
        return 0;
    } else  if(action == ACT_LEFT) {
        (*tb->pos)--;
        if (*tb->pos < 0) {
            *tb->pos = vector_size(&tb->options) -1;
        }
        component_toggle(c, *tb->pos);
        sound_play(20, 0.5f, -0.5f, 2.0f);
        return 0;
    }
    return 1;
}

void textselector_tick(component *c) {
    textselector *tb = c->obj;
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

void textselector_bindvar(component *c, int *var) {
    textselector *tb = c->obj;
    tb->pos = (var ? var : &tb->pos_);
}

int textselector_get_pos(component *c) {
    textselector *tb = c->obj;
    return *tb->pos;
}

void textselector_set_pos(component *c, int pos) {
    textselector *tb = c->obj;
    *tb->pos = pos;
}
