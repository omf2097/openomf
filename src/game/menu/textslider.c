#include "game/menu/textslider.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

void textslider_create(component *c, font *font, const char *text, unsigned int positions, int has_off) {
    component_create(c);
    textslider *tb;
    tb = malloc(sizeof(textslider));
    tb->text = text;
    tb->font = font;
    tb->ticks = 0;
    tb->dir = 0;
    tb->pos_ = 1;
    tb->pos = &tb->pos_;
    tb->has_off = has_off;
    tb->positions = positions;
    c->obj = tb;
    c->render = textslider_render;
    c->event = textslider_event;
    c->action = textslider_action;
    c->tick = textslider_tick;
}

void textslider_free(component *c) {
    textslider *tb = c->obj;
    free(tb);
    component_free(c);
}

void textslider_render(component *c) {
    textslider *tb = c->obj;
    char buf[100];
    int chars;
    int width;
    int xoff;
    sprintf(buf, "%s ", tb->text);
    chars = strlen(buf);
    if (tb->has_off && *tb->pos == 0) {
        buf[chars] = 'O';
        buf[chars+1] = 'F';
        buf[chars+2] = 'F';
        buf[chars+3] = '\0';
    } else {
        for(int i = 0; i < tb->positions; i++) {
            if (i+1 > *tb->pos) {
                buf[chars+i] = '|';
            } else {
                buf[chars+i] = 127;
            }
        }
        // null terminator
        buf[chars+tb->positions] = '\0';
    }
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

int textslider_event(component *c, SDL_Event *event) {
    return 1;
}

int textslider_action(component *c, int action) {
    textslider *tb = c->obj;
    if (action == ACT_KICK || action == ACT_PUNCH || action == ACT_RIGHT) {
        (*tb->pos)++;
        if (*tb->pos >= tb->positions) {
            *tb->pos = tb->positions;
        }
        if(c->slide != NULL) {
            c->slide(c, c->userdata, *tb->pos);
        }
        return 0;
    } else  if(action == ACT_LEFT) {
        (*tb->pos)--;
        if (tb->has_off && *tb->pos <= 0) {
            *tb->pos = 0;
        } else if (*tb->pos < 1) {
            *tb->pos = 1;
        }
        if(c->slide != NULL) {
            c->slide(c, c->userdata, *tb->pos);
        }
        return 0;
    }
    return 1;
}

void textslider_tick(component *c) {
    textslider *tb = c->obj;
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

void textslider_bindvar(component *c, int *var) {
    textslider *tb = c->obj;
    tb->pos = (var ? var : &tb->pos_);
}
