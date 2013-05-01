#ifndef _TEXTBUTTON_H
#define _TEXTBUTTON_H

#include "game/menu/component.h"
#include "game/text/text.h"

typedef struct textbutton_t textbutton;

struct textbutton_t {
    const char *text;
    font *font;
};

void textbutton_create(component *c, font *font, const char *text);
void textbutton_free(component *c);
void textbutton_render(component *c);
void textbutton_event(component *c);


#endif // _TEXTBUTTON_H