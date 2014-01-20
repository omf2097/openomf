#include "game/menu/textbutton.h"
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

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
    int chars = strlen(tb->text);
    int width = chars*tb->font->w;
    int xoff = (c->w - width)/2;
    if(c->selected) {
        int t = tb->ticks / 2;
        font_render(tb->font, tb->text, c->x + xoff, c->y, color_create(80 - t, 220 - t*2, 80 - t, 255));
    } else if (c->disabled) {
        font_render(tb->font, tb->text, c->x + xoff, c->y, color_create(121, 121, 121, 255));
    } else {
        font_render(tb->font, tb->text, c->x + xoff, c->y, color_create(0, 121, 0, 255));
    }
}

int textbutton_event(component *c, SDL_Event *event) {
    // Handle selection
    switch(event->type) {
        case SDL_KEYDOWN:
            if(event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER) {
                if(c->click != NULL) {
                    c->click(c, c->userdata);
                }
                return 0;
            }
    }
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
