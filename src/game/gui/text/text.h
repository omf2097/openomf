#ifndef TEXT_ENGINE_H
#define TEXT_ENGINE_H

#include <stdbool.h>
#include <stdint.h>

#include "game/gui/text/enums.h"
#include "resources/fonts.h"
#include "utils/str.h"
#include "video/vga_palette.h"

typedef struct text text;
typedef struct text_document text_document;

#define TEXT_BBOX_MAX UINT16_MAX

#define TEXT_YELLOW 0xF8
// EDIT: for shadows, the palette *one after* cshadow is used.
// so we set TEXT_SHADOW_YELLOW to 0xC0 and it renders in 0xC1.
#define TEXT_SHADOW_YELLOW 0xC0

#define CURSOR_STR "\x7f"
#define CURSOR_CHAR CURSOR_STR[0]

text *text_create(void);
text *text_create_with_font(font_size font);
text *text_create_with_font_and_size(font_size font, uint16_t w, uint16_t h);
text *text_create_from_c(const char *src);
text *text_create_from_str(const str *src);
void text_free(text **t);

text_document *text_document_create(void);
void text_document_free(text_document **d);

uint16_t text_document_get_text_count(text_document *d);
text *text_document_get_text(text_document *d, uint16_t index);

void text_set_from_c(text *t, const char *src);
void text_set_from_str(text *t, const str *src);

void text_set_font(text *t, font_size font);
void text_set_bounding_box(text *t, uint16_t w, uint16_t h);
void text_set_color(text *t, vga_index color);
void text_set_shadow_color(text *t, vga_index color);
void text_set_vertical_align(text *t, text_vertical_align align);
void text_set_horizontal_align(text *t, text_horizontal_align align);
void text_set_margin(text *t, const text_margin m);
void text_set_direction(text *t, text_row_direction direction);
void text_set_line_spacing(text *t, uint8_t line_spacing);
void text_set_letter_spacing(text *t, uint8_t letter_spacing);
void text_set_shadow_style(text *t, uint8_t shadow);
void text_set_glyph_margin(text *t, uint8_t glyph_margin);
void text_set_word_wrap(text *t, bool enable);

void text_get_str(const text *t, str *dst);
font_size text_get_font(const text *t);
void text_get_bounding_box(const text *t, uint16_t *w, uint16_t *h);
vga_index text_get_color(const text *t);
vga_index text_get_shadow_color(const text *t);
text_vertical_align text_get_vertical_align(const text *t);
text_horizontal_align text_get_horizontal_align(const text *t);
void text_get_margin(const text *t, uint8_t *left, uint8_t *right, uint8_t *top, uint8_t *bottom);
text_row_direction text_get_direction(const text *t);
uint8_t text_get_line_spacing(const text *t);
uint8_t text_get_letter_spacing(const text *t);
uint8_t text_get_shadow_style(const text *t);
uint8_t text_get_glyph_margin(const text *t);
bool text_get_word_wrap(const text *t);

/**
 * Get the maximum width of the rendered block of text. This may be smaller than the bounding box.
 * @warning Note that the text_generate_layout() or text_draw() must have been called first!
 */
uint16_t text_get_layout_width(const text *t);

/**
 * Get the maximum height of the rendered block of text. This may be smaller than the bounding box.
 * @warning Note that the text_generate_layout() or text_draw() must have been called first!
 */
uint16_t text_get_layout_height(const text *t);

/**
 * Get the amoutn of rows of the rendered block of text.
 * @warning Note that the text_generate_layout() or text_draw() must have been called first!
 */
size_t text_get_layout_rows(const text *t);

void text_generate_document(text_document *td, str *buf0, font_size font_sz, uint16_t w, uint16_t h,
                            vga_index text_color, vga_index shadow_color, text_vertical_align vertical_align,
                            text_horizontal_align horizontal_align, text_margin margin, uint8_t line_spacing,
                            uint8_t letter_spacing, uint8_t shadow, uint8_t glyph_margin);

/**
 * Immediately generate the text layout with all glyphs set at correct coordinates.
 * This will be run by the renderer at first screen render, if it has not yet been generated. This
 * function exists to allow generating texts at scene creation time.
 */
void text_generate_layout(text *t);

/**
 * Render the text document to given coordinates.
 */
void text_document_draw(text_document *d, int16_t offset_x, int16_t offset_y);

/**
 * Render the text block to given coordinates.
 */
void text_draw(text *t, int16_t x, int16_t y);

#endif // TEXT_ENGINE_H
