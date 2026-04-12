#ifndef SCREEN_SURFACE_H
#define SCREEN_SURFACE_H

#include "video/surface.h"
#include "video/vga_palette.h"

/**
 * Surface type for FBO readback with 1024-color (10-bit) palette index support.
 * Used as an intermediate format between GPU readback and conversion to 8-bit surface.
 * If we event want to do operations on software and write back, this can be used for that.
 */
typedef struct screen_surface {
    int w;
    int h;
    vga_index *data;
} screen_surface;

/**
 * Create a zeroed screen surface.
 *
 * @param sur Screen surface to initialize
 * @param w Width in pixels
 * @param h Height in pixels
 */
void screen_surface_create(screen_surface *sur, int w, int h);

/**
 * Create a screen surface by cloning an existing one.
 *
 * @param dst Destination screen surface to initialize
 * @param src Source screen surface to copy from
 */
void screen_surface_create_from(screen_surface *dst, const screen_surface *src);

/**
 * Create a screen surface from raw uint16_t pixel data, flipping vertically
 * and scaling values in one pass. Intended for GL framebuffer readback where
 * the Y-axis is inverted and values need to be scaled from 16-bit to 10-bit range.
 *
 * @param sur Screen surface to initialize
 * @param w Width in pixels
 * @param h Height in pixels
 * @param src Raw uint16_t pixel data from glReadPixels
 * @param scale Scale factor applied to each value (e.g. 1023.0f / 65535.0f)
 */
void screen_surface_create_from_u16_flip(screen_surface *sur, int w, int h, const uint16_t *src, float scale);

/**
 * Free screen surface data.
 *
 * @param sur Screen surface to free
 */
void screen_surface_free(screen_surface *sur);

/**
 * Convert screen surface to a grayscale 8-bit surface.
 * Uses the full 1024-entry palette for lookup, and maps each pixel to the closest
 * gray in the range [range_start, range_end]. Indices below ignore_below are passed through.
 *
 * @param src Source screen surface
 * @param dst Destination 8-bit surface to initialize
 * @param pal Palette to use for luminosity calculation
 * @param range_start First gray palette index
 * @param range_end Last gray palette index
 * @param ignore_below Leave indices below this value alone
 */
void screen_surface_to_grayscale(const screen_surface *src, surface *dst, const vga_palette *pal, vga_index range_start,
                                 vga_index range_end, int ignore_below);

#endif // SCREEN_SURFACE_H
