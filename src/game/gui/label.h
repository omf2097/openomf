#ifndef LABEL_H
#define LABEL_H

#include "game/gui/component.h"
#include "game/gui/text/text.h"

component *label_create_with_width(const char *text, uint16_t max_width);
component *label_create_title(const char *text);
component *label_create(const char *text);
void label_set_text(component *label, const char *text);
void label_set_text_color(component *label, vga_index color);
void label_set_text_horizontal_align(component *c, text_horizontal_align align);
void label_set_text_vertical_align(component *c, text_vertical_align align);
void label_set_text_letter_spacing(component *c, uint8_t spacing);
void label_set_font(component *label, font_size font);

#endif // LABEL_H
