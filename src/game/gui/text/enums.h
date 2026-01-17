/**
 * @file enums.h
 * @brief Text rendering enumerations
 * @details Enumerations for text alignment, direction, margins, and shadow styles.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef TEXT_ENUMS_H
#define TEXT_ENUMS_H

#include <stdint.h>

/**
 * @brief Vertical text alignment options
 */
typedef enum text_vertical_align
{
    TEXT_ALIGN_TOP,    ///< Align text to the top
    TEXT_ALIGN_MIDDLE, ///< Align text to the middle (vertically centered)
    TEXT_ALIGN_BOTTOM  ///< Align text to the bottom
} text_vertical_align;

/**
 * @brief Horizontal text alignment options
 */
typedef enum text_horizontal_align
{
    TEXT_ALIGN_LEFT,   ///< Align text to the left
    TEXT_ALIGN_CENTER, ///< Align text to the center (horizontally centered)
    TEXT_ALIGN_RIGHT   ///< Align text to the right
} text_horizontal_align;

/**
 * @brief Text margin values
 */
typedef struct text_margin {
    uint8_t left;   ///< Left margin in pixels
    uint8_t right;  ///< Right margin in pixels
    uint8_t top;    ///< Top margin in pixels
    uint8_t bottom; ///< Bottom margin in pixels
} text_margin;

/**
 * @brief Text row direction options
 */
typedef enum text_row_direction
{
    TEXT_ROW_HORIZONTAL, ///< Characters flow horizontally (left to right)
    TEXT_ROW_VERTICAL,   ///< Characters flow vertically (top to bottom)
} text_row_direction;

/**
 * @brief Glyph shadow direction flags
 */
typedef enum glyph_shadow
{
    GLYPH_SHADOW_NONE = 0,     ///< No shadow
    GLYPH_SHADOW_TOP = 0x1,    ///< Shadow above the glyph
    GLYPH_SHADOW_BOTTOM = 0x2, ///< Shadow below the glyph
    GLYPH_SHADOW_LEFT = 0x4,   ///< Shadow to the left of the glyph
    GLYPH_SHADOW_RIGHT = 0x8,  ///< Shadow to the right of the glyph
    GLYPH_SHADOW_ALL = 0xF     ///< Shadow in all directions
} glyph_shadow;

#endif // TEXT_ENUMS_H
