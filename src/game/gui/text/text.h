/**
 * @file text.h
 * @brief Text rendering engine
 * @details High-level text rendering with fonts, colors, alignment, and layout support.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
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

#define TEXT_BBOX_MAX UINT16_MAX ///< Maximum bounding box dimension

#ifndef TEXT_MEDIUM_GREEN
#define TEXT_DARK_GREEN 0xFE   ///< Dark green text color
#define TEXT_MEDIUM_GREEN 0xFE ///< Medium green text color
#define TEXT_BLINKY_GREEN 0xFF ///< Blinking green text color
#define TEXT_BRIGHT_GREEN 0xFD ///< Bright green text color
#define TEXT_TRN_BLUE 0xAB     ///< Tournament blue text color
#endif

#define TEXT_YELLOW 0xF8        ///< Yellow text color
#define TEXT_SHADOW_YELLOW 0xC0 ///< Yellow shadow color (palette offset, renders as 0xC1)

#define CURSOR_STR "\x7f"         ///< Cursor character as string
#define CURSOR_CHAR CURSOR_STR[0] ///< Cursor character

/**
 * @brief Create an empty text object
 * @return Pointer to the newly created text object
 */
text *text_create(void);

/**
 * @brief Create a text object with a specific font
 * @param font Font size to use
 * @return Pointer to the newly created text object
 */
text *text_create_with_font(font_size font);

/**
 * @brief Create a text object with a specific font and bounding box
 * @param font Font size to use
 * @param w Bounding box width
 * @param h Bounding box height
 * @return Pointer to the newly created text object
 */
text *text_create_with_font_and_size(font_size font, uint16_t w, uint16_t h);

/**
 * @brief Create a text object from a C string
 * @param src Source string
 * @return Pointer to the newly created text object
 */
text *text_create_from_c(const char *src);

/**
 * @brief Create a text object from a str object
 * @param src Source str object
 * @return Pointer to the newly created text object
 */
text *text_create_from_str(const str *src);

/**
 * @brief Clone a text object
 * @param src Text object to clone
 * @return Pointer to the cloned text object
 */
text *text_clone(const text *src);

/**
 * @brief Free a text object
 * @param t Pointer to text object pointer (will be set to NULL)
 */
void text_free(text **t);

/**
 * @brief Create a text document (multi-text container)
 * @return Pointer to the newly created document
 */
text_document *text_document_create(void);

/**
 * @brief Free a text document
 * @param d Pointer to document pointer (will be set to NULL)
 */
void text_document_free(text_document **d);

/**
 * @brief Get the number of text objects in a document
 * @param d Document to query
 * @return Number of text objects
 */
uint16_t text_document_get_text_count(text_document *d);

/**
 * @brief Get a text object from a document by index
 * @param d Document to query
 * @param index Index of the text object
 * @return Pointer to the text object
 */
text *text_document_get_text(text_document *d, uint16_t index);

/**
 * @brief Set text content from a C string
 * @param t Text object to modify
 * @param src Source string
 */
void text_set_from_c(text *t, const char *src);

/**
 * @brief Set text content from a str object
 * @param t Text object to modify
 * @param src Source str object
 */
void text_set_from_str(text *t, const str *src);

/**
 * @brief Set the font
 * @param t Text object to modify
 * @param font Font size to use
 */
void text_set_font(text *t, font_size font);

/**
 * @brief Set the bounding box dimensions
 * @param t Text object to modify
 * @param w Bounding box width
 * @param h Bounding box height
 */
void text_set_bounding_box(text *t, uint16_t w, uint16_t h);

/**
 * @brief Set the text color
 * @param t Text object to modify
 * @param color VGA palette color index
 */
void text_set_color(text *t, vga_index color);

/**
 * @brief Set the shadow color
 * @param t Text object to modify
 * @param color VGA palette color index for shadow
 */
void text_set_shadow_color(text *t, vga_index color);

/**
 * @brief Set the vertical alignment
 * @param t Text object to modify
 * @param align Vertical alignment value
 */
void text_set_vertical_align(text *t, text_vertical_align align);

/**
 * @brief Set the horizontal alignment
 * @param t Text object to modify
 * @param align Horizontal alignment value
 */
void text_set_horizontal_align(text *t, text_horizontal_align align);

/**
 * @brief Set the margin
 * @param t Text object to modify
 * @param m Margin values
 */
void text_set_margin(text *t, const text_margin m);

/**
 * @brief Set the text direction
 * @param t Text object to modify
 * @param direction Text row direction
 */
void text_set_direction(text *t, text_row_direction direction);

/**
 * @brief Set the line spacing
 * @param t Text object to modify
 * @param line_spacing Spacing in pixels between lines
 */
void text_set_line_spacing(text *t, uint8_t line_spacing);

/**
 * @brief Set the letter spacing
 * @param t Text object to modify
 * @param letter_spacing Spacing in pixels between letters
 */
void text_set_letter_spacing(text *t, uint8_t letter_spacing);

/**
 * @brief Set the shadow style
 * @param t Text object to modify
 * @param shadow Shadow direction flags (see glyph_shadow)
 */
void text_set_shadow_style(text *t, uint8_t shadow);

/**
 * @brief Set the glyph margin
 * @param t Text object to modify
 * @param glyph_margin Margin around each glyph in pixels
 */
void text_set_glyph_margin(text *t, uint8_t glyph_margin);

/**
 * @brief Set whether word wrapping is enabled
 * @param t Text object to modify
 * @param enable True to enable word wrapping
 */
void text_set_word_wrap(text *t, bool enable);

/**
 * @brief Get the text content as a str object
 * @param t Text object to query
 * @param dst Destination str object
 */
void text_get_str(const text *t, str *dst);

/**
 * @brief Get the text content as a C string
 * @param t Text object to query
 * @return Text content as C string
 */
const char *text_c(const text *t);

/**
 * @brief Get the font
 * @param t Text object to query
 * @return Font size
 */
font_size text_get_font(const text *t);

/**
 * @brief Get the bounding box dimensions
 * @param t Text object to query
 * @param w Output pointer for width
 * @param h Output pointer for height
 */
void text_get_bounding_box(const text *t, uint16_t *w, uint16_t *h);

/**
 * @brief Get the text color
 * @param t Text object to query
 * @return VGA palette color index
 */
vga_index text_get_color(const text *t);

/**
 * @brief Get the shadow color
 * @param t Text object to query
 * @return VGA palette color index for shadow
 */
vga_index text_get_shadow_color(const text *t);

/**
 * @brief Get the vertical alignment
 * @param t Text object to query
 * @return Vertical alignment value
 */
text_vertical_align text_get_vertical_align(const text *t);

/**
 * @brief Get the horizontal alignment
 * @param t Text object to query
 * @return Horizontal alignment value
 */
text_horizontal_align text_get_horizontal_align(const text *t);

/**
 * @brief Get the margin values
 * @param t Text object to query
 * @param left Output pointer for left margin
 * @param right Output pointer for right margin
 * @param top Output pointer for top margin
 * @param bottom Output pointer for bottom margin
 */
void text_get_margin(const text *t, uint8_t *left, uint8_t *right, uint8_t *top, uint8_t *bottom);

/**
 * @brief Get the text direction
 * @param t Text object to query
 * @return Text row direction
 */
text_row_direction text_get_direction(const text *t);

/**
 * @brief Get the line spacing
 * @param t Text object to query
 * @return Line spacing in pixels
 */
uint8_t text_get_line_spacing(const text *t);

/**
 * @brief Get the letter spacing
 * @param t Text object to query
 * @return Letter spacing in pixels
 */
uint8_t text_get_letter_spacing(const text *t);

/**
 * @brief Get the shadow style
 * @param t Text object to query
 * @return Shadow direction flags
 */
uint8_t text_get_shadow_style(const text *t);

/**
 * @brief Get the glyph margin
 * @param t Text object to query
 * @return Glyph margin in pixels
 */
uint8_t text_get_glyph_margin(const text *t);

/**
 * @brief Get whether word wrapping is enabled
 * @param t Text object to query
 * @return True if word wrapping is enabled
 */
bool text_get_word_wrap(const text *t);

/**
 * @brief Get the maximum width of the rendered text block
 * @warning text_generate_layout() or text_draw() must have been called first
 * @param t Text object to query
 * @return Width in pixels
 */
uint16_t text_get_layout_width(const text *t);

/**
 * @brief Get the maximum height of the rendered text block
 * @warning text_generate_layout() or text_draw() must have been called first
 * @param t Text object to query
 * @return Height in pixels
 */
uint16_t text_get_layout_height(const text *t);

/**
 * @brief Get the number of rows in the rendered text block
 * @warning text_generate_layout() or text_draw() must have been called first
 * @param t Text object to query
 * @return Number of rows
 */
size_t text_get_layout_rows(const text *t);

/**
 * @brief Generate a text document from raw parameters
 * @param td Text document to populate
 * @param buf0 Source text buffer
 * @param font_sz Font size
 * @param w Bounding box width
 * @param h Bounding box height
 * @param text_color Text color
 * @param shadow_color Shadow color
 * @param vertical_align Vertical alignment
 * @param horizontal_align Horizontal alignment
 * @param margin Text margin
 * @param line_spacing Line spacing
 * @param letter_spacing Letter spacing
 * @param shadow Shadow style flags
 * @param glyph_margin Glyph margin
 */
void text_generate_document(text_document *td, str *buf0, font_size font_sz, uint16_t w, uint16_t h,
                            vga_index text_color, vga_index shadow_color, text_vertical_align vertical_align,
                            text_horizontal_align horizontal_align, text_margin margin, uint8_t line_spacing,
                            uint8_t letter_spacing, uint8_t shadow, uint8_t glyph_margin);

/**
 * @brief Generate the text layout with all glyphs at correct coordinates
 * @details This will be run by the renderer at first screen render if not yet generated.
 *          This function exists to allow generating texts at scene creation time.
 * @param t Text object to generate layout for
 */
void text_generate_layout(text *t);

/**
 * @brief Render the text document to given coordinates
 * @param d Document to render
 * @param offset_x X offset for rendering
 * @param offset_y Y offset for rendering
 */
void text_document_draw(text_document *d, int16_t offset_x, int16_t offset_y);

/**
 * @brief Render the text block to given coordinates
 * @param t Text object to render
 * @param x X coordinate for rendering
 * @param y Y coordinate for rendering
 */
void text_draw(text *t, int16_t x, int16_t y);

#endif // TEXT_ENGINE_H
