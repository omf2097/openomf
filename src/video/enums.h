#ifndef VIDEO_ENUMS_H
#define VIDEO_ENUMS_H

typedef enum
{
    // If this is on, then we remap sprite using the selected index and blit.
    // Otherwise we use sprite palette indexes as remap selection indexes and modify existing image.
    SPRITE_REMAP = 0x01,
    // Render sprite at 1/4th height, and use four samples of its mask to generate a coverage
    SPRITE_SHADOW = 0x02,
    // This implements the "bg" tag feature. Add indexes together in the postprocess.
    // Requires FBUFOPT_CREDITS.
    SPRITE_INDEX_ADD = 0x04,
    // palette quirks for electra electricity and pyros fire
    SPRITE_HAR_QUIRKS = 0x08,
    // use weird stasis/shadow color blending
    SPRITE_DARK_TINT = 0x10,
} renderer_options;

typedef enum
{
    // Set when displaying the credits.
    // SPRITE_INDEX_ADD requires this, and is allowed to break
    // whatever effects aren't used on the credits.
    FBUFOPT_CREDITS = 0x01,
} renderer_framebuffer_options;

typedef enum
{
    FLIP_NONE = 0,
    FLIP_HORIZONTAL = 0x1,
    FLIP_VERTICAL = 0x2,
} video_flip_mode;

#endif // VIDEO_ENUMS_H
