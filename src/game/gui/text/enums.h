#ifndef TEXT_ENUMS_H
#define TEXT_ENUMS_H

#include <stdint.h>

typedef enum text_vertical_align
{
    TEXT_TOP = 0,
    TEXT_MIDDLE,
    TEXT_BOTTOM
} text_vertical_align;

typedef enum text_horizontal_align
{
    TEXT_LEFT = 0,
    TEXT_CENTER,
    TEXT_RIGHT
} text_horizontal_align;

typedef struct text_padding {
    uint8_t left;
    uint8_t right;
    uint8_t top;
    uint8_t bottom;
} text_padding;

typedef enum text_direction
{
    TEXT_HORIZONTAL = 0,
    TEXT_VERTICAL,
} text_direction;

typedef enum text_shadow
{
    TEXT_SHADOW_NONE = 0,
    TEXT_SHADOW_TOP = 0x1,
    TEXT_SHADOW_BOTTOM = 0x2,
    TEXT_SHADOW_LEFT = 0x4,
    TEXT_SHADOW_RIGHT = 0x8,
    TEXT_SHADOW_ALL = 0xF
} text_shadow;

#endif // TEXT_ENUMS_H
