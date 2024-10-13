#ifndef VIDEO_ENUMS_H
#define VIDEO_ENUMS_H

typedef enum
{
    BLEND_SET = 0,
    BLEND_ADD = 1,
    BLEND_SUB = 2,
} video_blend_mode;

typedef enum
{
    FLIP_NONE = 0,
    FLIP_HORIZONTAL = 0x1,
    FLIP_VERTICAL = 0x2,
} video_flip_mode;

#endif // VIDEO_ENUMS_H
