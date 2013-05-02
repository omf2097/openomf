#ifndef _TEXTBUTTON_H
#define _TEXTBUTTON_H

#include "game/menu/component.h"
#include "game/text/text.h"

typedef struct textbutton_t textbutton;

struct textbutton_t {
    const char *text;
    font *font;
    int ticks;
    int dir;
};

void textbutton_create(component *c, font *font, const char *text);
void textbutton_free(component *c);
void textbutton_render(component *c);
int textbutton_event(component *c, SDL_Event *event);
void textbutton_tick(component *c);

#endif // _TEXTBUTTON_H
