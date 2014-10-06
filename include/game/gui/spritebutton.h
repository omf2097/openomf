#ifndef _SPRITEBUTTON_H
#define _SPRITEBUTTON_H

#include "game/gui/component.h"
#include "game/text/text.h"
#include "video/surface.h"

enum {
    VALIGN_TOP = 0,
    VALIGN_MIDDLE,
    VALIGN_BOTTOM,
};

enum {
    HALIGN_LEFT = 0,
    HALIGN_CENTER,
    HALIGN_RIGHT
};

typedef void (*spritebutton_click_cb)(component *c, void *userdata);

component* spritebutton_create(const font *font, const char *text, surface *img, int disabled, spritebutton_click_cb cb, void *userdata);

void spritebutton_set_text_padding(component *c, int top, int bottom, int left, int right);
void spritebutton_set_text_align(component *c, int halign, int valign);

#endif // _SPRITEBUTTON_H
