#ifndef VIDEO_ENUMS_H
#define VIDEO_ENUMS_H

typedef enum
{
    BLEND_ADDITIVE = 0,
    BLEND_ALPHA = 1,
} VIDEO_BLEND_MODE;

typedef enum
{
    FLIP_NONE = 0,
    FLIP_HORIZONTAL = 0x1,
    FLIP_VERTICAL = 0x2,
} VIDEO_FLIP_MODE;

#endif // VIDEO_ENUMS_H
