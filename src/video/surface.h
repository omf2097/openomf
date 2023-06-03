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
    char *data;
    char *stencil;
    uint8_t force_refresh;
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
void surface_force_refresh(surface *sur);
void surface_create_from_vga(surface *sur, const sd_vga_image *src);
void surface_create_from_image(surface *sur, image *img);
void surface_create_from_data(surface *sur, int type, int w, int h, const char *src);
void surface_create_from_data_flip(surface *sur, int type, int w, int h, const char *src);
int surface_to_image(surface *sur, image *img);
void surface_copy(surface *dst, surface *src);
void surface_copy_ex(surface *dst, surface *src);
void surface_free(surface *sur);
void surface_clear(surface *sur);
void surface_fill(surface *sur, color c);
void surface_sub(surface *dst, surface *src, int dst_x, int dst_y, int src_x, int src_y, int w, int h, int method);
void surface_convert_to_rgba(surface *sur, screen_palette *pal, int pal_offset);

/**
 * Convert surface to grayscale using colors in palette from range-start to range-end (inclusive).
 * Conversion is done by luminosity. Surface must be paletted.
 * @param sur Surface to convert
 * @param pal Palette to use.
 * @param range_start First grey palette color
 * @param range_end Last gray palette color
 */
void surface_convert_to_grayscale(surface *sur, screen_palette *pal, int range_start, int range_end);
int surface_get_type(surface *sur);
void surface_to_rgba(surface *sur, char *dst, screen_palette *pal, char *remap_table, uint8_t pal_offset);
void surface_additive_blit(surface *dst, surface *src, int dst_x, int dst_y, palette *remap_pal, SDL_RendererFlip flip);
void surface_rgba_blit(surface *dst, const surface *src, int dst_x, int dst_y);
void surface_alpha_blit(surface *dst, surface *src, int dst_x, int dst_y, SDL_RendererFlip flip);
bool surface_write_png(surface *sur, screen_palette *pal, const char *filename);

#endif // SURFACE_H
