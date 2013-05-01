#ifndef _TEXTSELECTOR_H
#define _TEXTSELECTOR_H

#include "game/menu/component.h"
#include "game/text/text.h"

typedef struct textselector_t textselector;

struct textselector_t {
    const char *text;
    font *font;
    int ticks;
    int dir;
    int pos;
    int option_count;
    char *options[10];
};

void textselector_create(component *c, font *font, const char *text, const char *initialoption);
void textselector_add_option(component *c, const char *option);
void textselector_free(component *c);
void textselector_render(component *c);
int textselector_event(component *c, SDL_Event *event);
void textselector_tick(component *c);

#endif // _TEXTSELECTOR_H
