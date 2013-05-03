#include "game/menu/textselector.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

void textselector_create(component *c, font *font, const char *text, const char *initialvalue) {
    component_create(c);
    textselector *tb;
    tb = malloc(sizeof(textselector));
    tb->text = text;
    tb->font = font;
    tb->ticks = 0;
    tb->dir = 0;
    tb->pos = 0;
    vector_create(&tb->options, sizeof(char*));
    vector_append(&tb->options, &initialvalue);
    c->obj = tb;
    c->render = textselector_render;
    c->event = textselector_event;
    c->tick = textselector_tick;
}

void textselector_add_option(component *c, const char *value) {
    textselector *tb = c->obj;
    vector_append(&tb->options, &value);
}

void textselector_free(component *c) {
    textselector *tb = c->obj;
    vector_free(&tb->options);
    free(tb);
    component_free(c);
}

void textselector_render(component *c) {
    textselector *tb = c->obj;
    char buf[100];
    int chars;
    int width;
    int xoff;
    char **opt = vector_get(&tb->options, tb->pos);
    sprintf(buf, "%s %s", tb->text, *opt);
    chars = strlen(buf);
    width = chars*tb->font->w;
    xoff = (c->w - width)/2;
    if(c->selected) {
        int t = tb->ticks / 2;
        font_render(tb->font, buf, c->x + xoff, c->y, 80 - t, 220 - t*2, 80 - t);
    } else if (c->disabled) {
        font_render(tb->font, buf, c->x + xoff, c->y, 121, 121, 121);
    } else {
        font_render(tb->font, buf, c->x + xoff, c->y, 0, 121, 0);
    }
}

int textselector_event(component *c, SDL_Event *event) {
    // Handle selection
    textselector *tb = c->obj;
    switch(event->type) {
        case SDL_KEYDOWN:
            if(event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_RIGHT) {
                tb->pos++;
                if (tb->pos >= vector_size(&tb->options)) {
                    tb->pos = 0;
                }
                if(c->toggle != NULL) {
                    c->toggle(c, c->userdata, tb->pos);
                }
                return 0;
            } else  if(event->key.keysym.sym == SDLK_LEFT) {
                tb->pos--;
                if (tb->pos < 0) {
                    tb->pos = vector_size(&tb->options) -1;
                }
                if(c->toggle != NULL) {
                    c->toggle(c, c->userdata, tb->pos);
                }
                return 0;
            }
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
int textselector_getpos(component *c) {
    textselector *tb = c->obj;
    return tb->pos;
}
void textselector_setpos(component *c, int pos) {
    textselector *tb = c->obj;
    tb->pos = pos;
}
