#ifndef OMF_FRAMEBUFFER_PRESENTER_H
#define OMF_FRAMEBUFFER_PRESENTER_H

#include "utils/rect.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Numeric pixel value 0xAABBGGRR (RGBA bytes on little-endian targets). */
typedef enum framebuffer_pixel_format
{
    FRAMEBUFFER_FORMAT_ABGR8888,
} framebuffer_pixel_format;

/* Pixels remain owned by the caller and must be valid until present() returns. */
typedef struct framebuffer_image {
    const void *pixels;
    int width;
    int height;
    size_t pitch; // Bytes per row, including any padding.
    framebuffer_pixel_format format;
} framebuffer_image;

/*
 * Display a completed CPU framebuffer. The presenter owns its display
 * resources; it does not own the source image or perform any compositing.
 * Destination coordinates are in output pixels and may be partly offscreen.
 */
typedef struct framebuffer_presenter {
    void *ctx;
    const char *(*get_name)(void);
    bool (*open)(void *ctx, int output_width, int output_height, bool fullscreen, bool vsync, int scaling_mode);
    bool (*present)(void *ctx, const framebuffer_image *image, const irect *destination);
    void (*close)(void *ctx);
    void (*destroy)(void *ctx);
} framebuffer_presenter;

/* Validate format, dimensions, stride and non-null storage. */
bool framebuffer_image_valid(const framebuffer_image *image);

/* Backends are selected independently of the game's OpenGL3/Software renderer. */
int framebuffer_presenter_count(void);
const char *framebuffer_presenter_name(int index);
bool framebuffer_presenter_create(framebuffer_presenter *presenter, const char *preferred_name);
void framebuffer_presenter_destroy(framebuffer_presenter *presenter);

#endif // OMF_FRAMEBUFFER_PRESENTER_H
