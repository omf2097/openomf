/**
 * @file text_layout.h
 * @brief Text layout computation
 * @details Low-level text layout calculation for positioning glyphs within bounding boxes.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef TEXT_LAYOUT_H
#define TEXT_LAYOUT_H

#include "game/gui/text/enums.h"
#include "resources/fonts.h"
#include "utils/str.h"
#include "utils/vector.h"
#include "video/surface.h"

/**
 * @brief Single glyph position in the layout
 */
typedef struct text_layout_item {
    uint16_t x;           ///< Relative X coordinate within the bounding box
    uint16_t y;           ///< Relative Y coordinate within the bounding box
    const surface *glyph; ///< Pointer to the glyph surface
} text_layout_item;

/**
 * @brief Complete text layout with all positioned glyphs
 */
typedef struct text_layout {
    vector items; ///< Vector of text_layout_item
    size_t rows;  ///< Total rows rendered
    uint16_t w;   ///< Total width of rendered items
    uint16_t h;   ///< Total height of rendered items
} text_layout;

/**
 * @brief Initialize a text layout structure
 * @param layout Layout structure to initialize
 */
void text_layout_create(text_layout *layout);

/**
 * @brief Clone a text layout
 * @param dst Destination layout structure
 * @param src Source layout to clone
 */
void text_layout_clone(text_layout *dst, const text_layout *src);

/**
 * @brief Free a text layout's resources
 * @param layout Layout to free
 */
void text_layout_free(text_layout *layout);

/**
 * @brief Find the end index of the next line in the text
 * @details Jumps over a line of text, and finds the starting index of the next line.
 *          If no text could fit into the given space, start_index is returned.
 * @param buf Text buffer to search
 * @param font Font being used
 * @param direction Text direction
 * @param start_index Starting character index
 * @param letter_spacing Letter spacing in pixels
 * @param max_width Maximum line width in pixels
 * @param word_wrap Whether to wrap at word boundaries
 * @return Character index where the line should end, or start_index if text doesn't fit
 */
size_t find_next_line_end(const str *buf, const font *font, text_row_direction direction, size_t start_index,
                          uint8_t letter_spacing, uint16_t max_width, bool word_wrap);

/**
 * @brief Compute the full text layout
 * @param layout Layout structure to populate after computation
 * @param buf Text buffer to layout
 * @param font Font to use for rendering
 * @param bbox_w Bounding box width (affects word wrapping)
 * @param bbox_h Bounding box height (affects word wrapping)
 * @param vertical_align Vertical alignment within bounding box
 * @param horizontal_align Horizontal alignment within bounding box
 * @param margin Bounding box margins
 * @param direction Text rendering direction (left to right or top to bottom)
 * @param line_spacing Spacing between lines in pixels
 * @param letter_spacing Spacing between letters in pixels
 * @param word_wrap Whether to enable word wrapping
 */
void text_layout_compute(text_layout *layout, const str *buf, const font *font, uint16_t bbox_w, uint16_t bbox_h,
                         text_vertical_align vertical_align, text_horizontal_align horizontal_align, text_margin margin,
                         text_row_direction direction, uint8_t line_spacing, uint8_t letter_spacing, bool word_wrap);

#endif // TEXT_LAYOUT_H
