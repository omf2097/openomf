/*! @file
 * @brief Text layout computation for glyph positioning.
 */

#ifndef TEXT_LAYOUT_H
#define TEXT_LAYOUT_H

#include "game/gui/text/enums.h"
#include "resources/fonts.h"
#include "utils/str.h"
#include "utils/vector.h"
#include "video/surface.h"

/**
 * @brief A single positioned glyph in the layout.
 */
typedef struct text_layout_item {
    uint16_t x;           /**< Relative X coordinate within the bounding box */
    uint16_t y;           /**< Relative Y coordinate within the bounding box */
    const surface *glyph; /**< Pointer to the glyph surface (owned by font) */
} text_layout_item;

/**
 * @brief Computed layout containing positioned glyphs.
 */
typedef struct text_layout {
    vector items; /**< Vector of text_layout_item */
    size_t rows;  /**< Total rows rendered */
    uint16_t w;   /**< Total width of rendered items */
    uint16_t h;   /**< Total height of rendered items */
} text_layout;

/**
 * @brief Initialize a text layout structure.
 * @param layout Layout to initialize.
 */
void text_layout_create(text_layout *layout);

/**
 * @brief Clone a text layout.
 * @param dst Destination layout (must be uninitialized or freed).
 * @param src Source layout to clone.
 */
void text_layout_clone(text_layout *dst, const text_layout *src);

/**
 * @brief Free a text layout's resources.
 * @param layout Layout to free.
 */
void text_layout_free(text_layout *layout);

/**
 * @brief Find the end index of the next line in the text buffer.
 * @param buf Text buffer to scan.
 * @param font Font to use for glyph widths.
 * @param direction Row direction (left-to-right or right-to-left).
 * @param start_index Starting character index.
 * @param letter_spacing Extra spacing between characters.
 * @param max_width Maximum line width in pixels.
 * @param word_wrap Whether to break at word boundaries.
 * @return Index of the last character in the line.
 */
size_t find_next_line_end(const str *buf, const font *font, text_row_direction direction, size_t start_index,
                          uint8_t letter_spacing, uint16_t max_width, bool word_wrap);

/**
 * @brief Compute the layout for text rendering.
 * @param layout Output layout structure.
 * @param buf Text content to layout.
 * @param font Font to use.
 * @param bbox_w Bounding box width.
 * @param bbox_h Bounding box height.
 * @param vertical_align Vertical alignment within bounding box.
 * @param horizontal_align Horizontal alignment within bounding box.
 * @param margin Margins inside bounding box.
 * @param direction Row direction.
 * @param line_spacing Extra spacing between lines.
 * @param letter_spacing Extra spacing between characters.
 * @param word_wrap Whether to wrap at word boundaries.
 */
void text_layout_compute(text_layout *layout, const str *buf, const font *font, uint16_t bbox_w, uint16_t bbox_h,
                         text_vertical_align vertical_align, text_horizontal_align horizontal_align, text_margin margin,
                         text_row_direction direction, uint8_t line_spacing, uint8_t letter_spacing, bool word_wrap);

#endif // TEXT_LAYOUT_H
