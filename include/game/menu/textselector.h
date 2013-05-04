#ifndef _TEXTSELECTOR_H
#define _TEXTSELECTOR_H

#include "game/menu/component.h"
#include "game/text/text.h"
#include "utils/vector.h"

typedef struct textselector_t textselector;

struct textselector_t {
    const char *text;
    font *font;
    int ticks;
    int dir;
    int pos_;
    int *pos;
    vector options;
    void(*clicked)(textselector*);
};

void textselector_create(component *c, font *font, const char *text, const char *initialoption);
void textselector_add_option(component *c, const char *option);
void textselector_free(component *c);
void textselector_render(component *c);
int textselector_event(component *c, SDL_Event *event);
void textselector_tick(component *c);
void textselector_bindvar(component *c, int *var);
void textselector_bindclicked(component *c, void(*clicked)(textselector*));

#endif // _TEXTSELECTOR_H
