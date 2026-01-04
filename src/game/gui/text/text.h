/*! @file
 * @brief Text rendering engine with layout, styling, and multi-document support.
 */

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

// For compatibility
#ifndef TEXT_MEDIUM_GREEN
#define TEXT_DARK_GREEN 0xFE
#define TEXT_MEDIUM_GREEN 0xFE
#define TEXT_BLINKY_GREEN 0xFF
#define TEXT_BRIGHT_GREEN 0xFD
#define TEXT_TRN_BLUE 0xAB
#endif

#define TEXT_YELLOW 0xF8
// EDIT: for shadows, the palette *one after* cshadow is used.
// so we set TEXT_SHADOW_YELLOW to 0xC0 and it renders in 0xC1.
#define TEXT_SHADOW_YELLOW 0xC0

#define CURSOR_STR "\x7f"
#define CURSOR_CHAR CURSOR_STR[0]

/** @brief Create a text object with default settings. */
text *text_create(void);

/** @brief Create a text object with the specified font. */
text *text_create_with_font(font_size font);

/** @brief Create a text object with font and bounding box. */
text *text_create_with_font_and_size(font_size font, uint16_t w, uint16_t h);

/** @brief Create a text object from a C string. */
text *text_create_from_c(const char *src);

/** @brief Create a text object from a str. */
text *text_create_from_str(const str *src);

/** @brief Clone a text object. */
text *text_clone(const text *src);

/** @brief Free a text object. */
void text_free(text **t);

/** @brief Create a text document container. */
text_document *text_document_create(void);

/** @brief Free a text document. */
void text_document_free(text_document **d);

/** @brief Get the number of text blocks in the document. */
uint16_t text_document_get_text_count(text_document *d);

/** @brief Get a text block by index. */
text *text_document_get_text(text_document *d, uint16_t index);

/** @brief Set the text content from a C string. */
void text_set_from_c(text *t, const char *src);

/** @brief Set the text content from a str. */
void text_set_from_str(text *t, const str *src);

/** @brief Set the font. */
void text_set_font(text *t, font_size font);

/** @brief Set the bounding box dimensions. */
void text_set_bounding_box(text *t, uint16_t w, uint16_t h);

/** @brief Set the text color. */
void text_set_color(text *t, vga_index color);

/** @brief Set the shadow color. */
void text_set_shadow_color(text *t, vga_index color);

/** @brief Set vertical alignment. */
void text_set_vertical_align(text *t, text_vertical_align align);

/** @brief Set horizontal alignment. */
void text_set_horizontal_align(text *t, text_horizontal_align align);

/** @brief Set margins. */
void text_set_margin(text *t, const text_margin m);

/** @brief Set row direction. */
void text_set_direction(text *t, text_row_direction direction);

/** @brief Set line spacing. */
void text_set_line_spacing(text *t, uint8_t line_spacing);

/** @brief Set letter spacing. */
void text_set_letter_spacing(text *t, uint8_t letter_spacing);

/** @brief Set shadow style bitmask. */
void text_set_shadow_style(text *t, uint8_t shadow);

/** @brief Set glyph margin. */
void text_set_glyph_margin(text *t, uint8_t glyph_margin);

/** @brief Enable or disable word wrapping. */
void text_set_word_wrap(text *t, bool enable);

/** @brief Get the text content as a str. */
void text_get_str(const text *t, str *dst);

/** @brief Get the text content as a C string. */
const char *text_c(const text *t);

/** @brief Get the font. */
font_size text_get_font(const text *t);

/** @brief Get the bounding box dimensions. */
void text_get_bounding_box(const text *t, uint16_t *w, uint16_t *h);

/** @brief Get the text color. */
vga_index text_get_color(const text *t);

/** @brief Get the shadow color. */
vga_index text_get_shadow_color(const text *t);

/** @brief Get vertical alignment. */
text_vertical_align text_get_vertical_align(const text *t);

/** @brief Get horizontal alignment. */
text_horizontal_align text_get_horizontal_align(const text *t);

/** @brief Get margins. */
void text_get_margin(const text *t, uint8_t *left, uint8_t *right, uint8_t *top, uint8_t *bottom);

/** @brief Get row direction. */
text_row_direction text_get_direction(const text *t);

/** @brief Get line spacing. */
uint8_t text_get_line_spacing(const text *t);

/** @brief Get letter spacing. */
uint8_t text_get_letter_spacing(const text *t);

/** @brief Get shadow style bitmask. */
uint8_t text_get_shadow_style(const text *t);

/** @brief Get glyph margin. */
uint8_t text_get_glyph_margin(const text *t);

/** @brief Check if word wrapping is enabled. */
bool text_get_word_wrap(const text *t);

/**
 * @brief Get the rendered text width.
 * @warning Requires text_generate_layout() or text_draw() to be called first.
 */
uint16_t text_get_layout_width(const text *t);

/**
 * @brief Get the rendered text height.
 * @warning Requires text_generate_layout() or text_draw() to be called first.
 */
uint16_t text_get_layout_height(const text *t);

/**
 * @brief Get the number of rendered text rows.
 * @warning Requires text_generate_layout() or text_draw() to be called first.
 */
size_t text_get_layout_rows(const text *t);

/** @brief Generate a document from text with specified formatting. */
void text_generate_document(text_document *td, str *buf0, font_size font_sz, uint16_t w, uint16_t h,
                            vga_index text_color, vga_index shadow_color, text_vertical_align vertical_align,
                            text_horizontal_align horizontal_align, text_margin margin, uint8_t line_spacing,
                            uint8_t letter_spacing, uint8_t shadow, uint8_t glyph_margin);

/**
 * @brief Pre-generate the text layout.
 * @details Positions all glyphs at their final coordinates. The renderer will
 *          call this automatically if not already done. Use this to generate
 *          layout at scene creation time.
 */
void text_generate_layout(text *t);

/** @brief Render a text document at the specified position. */
void text_document_draw(text_document *d, int16_t offset_x, int16_t offset_y);

/** @brief Render text at the specified position. */
void text_draw(text *t, int16_t x, int16_t y);

#endif // TEXT_ENGINE_H
