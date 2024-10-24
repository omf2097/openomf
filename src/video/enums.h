#ifndef VIDEO_ENUMS_H
#define VIDEO_ENUMS_H

typedef enum
{
    // If this is on, then we remap sprite using the selected index and blit.
    // Otherwise we use sprite palette indexes as remap selection indexes and modify existing image.
    REMAP_SPRITE = 0x01,
} renderer_options;

typedef enum
{
    FLIP_NONE = 0,
    FLIP_HORIZONTAL = 0x1,
    FLIP_VERTICAL = 0x2,
} video_flip_mode;

#endif // VIDEO_ENUMS_H
