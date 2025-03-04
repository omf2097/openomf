#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"

typedef void (*textbutton_click_cb)(component *c, void *userdata);

component *textbutton_create(const font_size font, const char *text, const char *help, int disabled,
                             textbutton_click_cb cb, void *userdata);
void textbutton_set_border(component *c, vga_index color);
void textbutton_set_text(component *c, const char *text);
void textbutton_set_userdata(component *c, void *userdata);

void textbutton_set_default_color(component *c, vga_index color);
void textbutton_set_selected_color(component *c, vga_index color);
void textbutton_set_disabled_color(component *c, vga_index color);

#endif // TEXTBUTTON_H
