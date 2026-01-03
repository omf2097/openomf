#ifndef SPRITEBUTTON_H
#define SPRITEBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"
#include "video/surface.h"

typedef void (*spritebutton_click_cb)(component *c, void *userdata);
typedef void (*spritebutton_tick_cb)(component *c, void *userdata);
typedef void (*spritebutton_focus_cb)(component *c, bool focused, void *userdata);
typedef component *(*spritebutton_find_text_cb)(component *c, const char *text);

component *spritebutton_create(const char *text, const surface *img, bool disabled, spritebutton_click_cb cb,
                               void *userdata);

void spritebutton_set_horizontal_align(component *c, text_horizontal_align align);
void spritebutton_set_vertical_align(component *c, text_vertical_align align);
void spritebutton_set_text_direction(component *c, text_row_direction direction);
void spritebutton_set_text_margin(component *c, text_margin margins);
void spritebutton_set_text_color(component *c, vga_index color);
void spritebutton_set_font(component *c, font_size font);

const surface *spritebutton_get_img(const component *c);
void spritebutton_set_img(component *c, const surface *img);

void spritebutton_set_tick_cb(component *c, spritebutton_tick_cb);
void spritebutton_set_focus_cb(component *c, spritebutton_focus_cb);

void spritebutton_set_always_display(component *c);
void spritebutton_set_free_userdata(component *c, bool free_userdata);

#endif // SPRITEBUTTON_H
