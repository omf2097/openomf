/*! @file
 * @brief Text alignment, margin, and direction enumerations.
 */

#ifndef TEXT_ENUMS_H
#define TEXT_ENUMS_H

#include <stdint.h>

/** @brief Vertical text alignment options. */
typedef enum text_vertical_align
{
    TEXT_ALIGN_TOP,    ///< Align text to top of bounding box.
    TEXT_ALIGN_MIDDLE, ///< Center text vertically in bounding box.
    TEXT_ALIGN_BOTTOM  ///< Align text to bottom of bounding box.
} text_vertical_align;

/** @brief Horizontal text alignment options. */
typedef enum text_horizontal_align
{
    TEXT_ALIGN_LEFT,   ///< Align text to left of bounding box.
    TEXT_ALIGN_CENTER, ///< Center text horizontally in bounding box.
    TEXT_ALIGN_RIGHT   ///< Align text to right of bounding box.
} text_horizontal_align;

/** @brief Text margin configuration. */
typedef struct text_margin {
    uint8_t left;   ///< Left margin in pixels.
    uint8_t right;  ///< Right margin in pixels.
    uint8_t top;    ///< Top margin in pixels.
    uint8_t bottom; ///< Bottom margin in pixels.
} text_margin;

/** @brief Text row direction. */
typedef enum text_row_direction
{
    TEXT_ROW_HORIZONTAL, ///< Standard left-to-right, top-to-bottom text flow.
    TEXT_ROW_VERTICAL,   ///< Vertical text flow (top-to-bottom per column).
} text_row_direction;

/** @brief Glyph shadow directions as a bitmask. */
typedef enum glyph_shadow
{
    GLYPH_SHADOW_NONE = 0,     ///< No shadow.
    GLYPH_SHADOW_TOP = 0x1,    ///< Shadow above the glyph.
    GLYPH_SHADOW_BOTTOM = 0x2, ///< Shadow below the glyph.
    GLYPH_SHADOW_LEFT = 0x4,   ///< Shadow to the left of the glyph.
    GLYPH_SHADOW_RIGHT = 0x8,  ///< Shadow to the right of the glyph.
    GLYPH_SHADOW_ALL = 0xF     ///< Shadow on all sides.
} glyph_shadow;

#endif // TEXT_ENUMS_H
