#include "game/menu/textbutton.h"
#include <stdlib.h>
#include "utils/log.h"

void textbutton_create(component *c, font *font, const char *text) {
    component_create(c);
    textbutton *tb;
    tb = malloc(sizeof(textbutton));
    tb->text = text;
    tb->font = font;
    tb->ticks = 0;
    tb->dir = 0;
    c->obj = tb;
    c->render = textbutton_render;
    c->event = textbutton_event;
    c->tick = textbutton_tick;
}

void textbutton_free(component *c) {
    textbutton *tb = c->obj;
    free(tb);
    component_free(c);
}

void textbutton_render(component *c) {
    textbutton *tb = c->obj;
    if(c->selected) {
        int t = tb->ticks / 2;
        font_render(tb->font, tb->text, c->x, c->y, 80 - t, 220 - t*2, 80 - t);
    } else {
        font_render(tb->font, tb->text, c->x, c->y, 80, 220, 80);
    }
}

int textbutton_event(component *c) {
    return 1;
}

void textbutton_tick(component *c) {
    textbutton *tb = c->obj;
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
