#ifndef SPRITEBUTTON_H
#define SPRITEBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"
#include "video/surface.h"

enum
{
    VALIGN_TOP = 0,
    VALIGN_MIDDLE,
    VALIGN_BOTTOM,
};

enum
{
    HALIGN_LEFT = 0,
    HALIGN_CENTER,
    HALIGN_RIGHT
};

typedef void (*spritebutton_click_cb)(component *c, void *userdata);
typedef void (*spritebutton_tick_cb)(component *c, void *userdata);

component *spritebutton_create(const text_settings *tconf, const char *text, surface *img, int disabled,
                               spritebutton_click_cb cb, void *userdata);

void spritebutton_set_tick_cb(component *c, spritebutton_tick_cb);

#endif // SPRITEBUTTON_H
