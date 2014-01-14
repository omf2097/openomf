#ifndef _TEXTINPUT_H
#define _TEXTINPUT_H

#include "game/menu/component.h"
#include "game/text/text.h"
#include "utils/vector.h"
#include "video/video.h"
#include "video/color.h"
#include "video/image.h"


typedef struct textinput_t textinput;

struct textinput_t {
    const char *text;
    font *font;
    int ticks;
    int dir;
    int pos_;
    int *pos;
    surface sur;
    char buf[50];
};

void textinput_create(component *c, font *font, const char *text, const char *initialoption);
void textinput_free(component *c);
void textinput_render(component *c);
int textinput_event(component *c, SDL_Event *event);
void textinput_tick(component *c);
char* textinput_value(component *c);

#endif // _TEXTINPUT_H
