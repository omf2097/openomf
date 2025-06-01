#ifndef TEXT_LAYOUT_H
#define TEXT_LAYOUT_H

#include "game/gui/text/enums.h"
#include "resources/fonts.h"
#include "utils/str.h"
#include "utils/vector.h"
#include "video/surface.h"

typedef struct text_layout_item {
    uint16_t x; // Relative X coordinate within the bounding box
    uint16_t y; // Relative Y coordinate within the bounding box
    surface *glyph;
} text_layout_item;

typedef struct text_layout {
    vector items;
    size_t rows; // Total rows rendered
    uint16_t w;  // Total width of rendered items.
    uint16_t h;  // Total height of rendered items.
} text_layout;

void text_layout_create(text_layout *layout);
void text_layout_clone(text_layout *dst, const text_layout *src);
void text_layout_free(text_layout *layout);
size_t find_next_line_end(const str *buf, const font *font, text_row_direction direction, size_t start_index,
                          uint8_t letter_spacing, uint16_t max_width, bool word_wrap);
void text_layout_compute(text_layout *layout, const str *buf, const font *font, uint16_t bbox_w, uint16_t bbox_h,
                         text_vertical_align vertical_align, text_horizontal_align horizontal_align, text_margin margin,
                         text_row_direction direction, uint8_t line_spacing, uint8_t letter_spacing, bool word_wrap);

#endif // TEXT_LAYOUT_H
