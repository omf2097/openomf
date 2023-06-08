#ifndef SURFACE_H
#define SURFACE_H

#include "formats/palette.h"
#include "formats/vga_image.h"
#include "video/image.h"
#include "video/screen_palette.h"
#include <SDL.h>

typedef struct {
    uint32_t hash;
    int w;
    int h;
    int type;
    unsigned char *data;
    unsigned char *stencil;
} surface;

enum
{
    SURFACE_TYPE_RGBA,
    SURFACE_TYPE_PALETTE
};

enum
{
    SUB_METHOD_NONE,
    SUB_METHOD_MIRROR
};

void surface_create(surface *sur, int type, int w, int h);
void surface_create_from(surface *dst, const surface *src);
void surface_create_from_vga(surface *sur, const sd_vga_image *src);
void surface_create_from_image(surface *sur, image *img);
void surface_create_from_data(surface *sur, int type, int w, int h, const unsigned char *src);
void surface_create_from_data_flip(surface *sur, int type, int w, int h, const unsigned char *src);
void surface_create_from_surface(surface *sur, int type, int w, int h, int src_x, int src_y, const surface *src);
int surface_to_image(surface *sur, image *img);
void surface_free(surface *sur);
void surface_clear(surface *sur);
void surface_fill(surface *sur, color c);
void surface_sub(surface *dst, const surface *src, int dst_x, int dst_y, int src_x, int src_y, int w, int h,
                 int method);
void surface_generate_stencil(const surface *sur, int index);

/**
 * Convert surface to grayscale using colors in palette from range-start to range-end (inclusive).
 * Conversion is done by luminosity. Surface must be paletted.
 * @param sur Surface to convert
 * @param pal Palette to use.
 * @param range_start First grey palette color
 * @param range_end Last gray palette color
 */
void surface_convert_to_grayscale(surface *sur, screen_palette *pal, int range_start, int range_end);

/**
 * Write surface to a PNG file. If surface is paletted, a paletted PNG is also used.
 *
 * @param sur Source surface
 * @param pal Palette to use (only applied if surface is paletted)
 * @param filename Target filename to write
 * @return True on success, false on any failure.
 */
bool surface_write_png(const surface *sur, screen_palette *pal, const char *filename);

#endif // SURFACE_H
