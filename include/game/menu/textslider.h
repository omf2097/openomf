#ifndef _TEXTSLIDER_H
#define _TEXTSLIDER_H

#include "game/menu/component.h"
#include "game/text/text.h"

typedef struct textslider_t textslider;

struct textslider_t {
    const char *text;
    font *font;
    int ticks;
    int dir;
    int pos;
    int positions;
};

void textslider_create(component *c, font *font, const char *text, unsigned int positions);
void textslider_free(component *c);
void textslider_render(component *c);
int textslider_event(component *c, SDL_Event *event);
void textslider_tick(component *c);
int textslider_getpos(component *c);
void textslider_setpos(component *c, int pos);

#endif // _TEXTSLIDER_H
