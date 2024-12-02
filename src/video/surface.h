#ifndef SURFACE_H
#define SURFACE_H

#include "formats/vga_image.h"
#include "video/image.h"
#include "video/vga_palette.h"
#include <SDL.h>

typedef struct {
    unsigned int guid;
    int w;
    int h;
    int transparent;
    unsigned char *data;
} surface;

enum
{
    SUB_METHOD_NONE,
    SUB_METHOD_MIRROR
};

void surface_create(surface *sur, int w, int h);
void surface_create_from(surface *dst, const surface *src);
void surface_create_from_vga(surface *sur, const sd_vga_image *src);
void surface_create_from_image(surface *sur, image *img);
void surface_create_from_data(surface *sur, int w, int h, const unsigned char *src);
void surface_create_from_data_flip(surface *sur, int w, int h, const unsigned char *src);
void surface_create_from_surface(surface *sur, int w, int h, int src_x, int src_y, const surface *src);
int surface_to_image(const surface *sur, image *img);
void surface_free(surface *sur);
void surface_clear(surface *sur);
void surface_sub(surface *dst, const surface *src, int dst_x, int dst_y, int src_x, int src_y, int w, int h,
                 int method);
void surface_set_transparency(surface *dst, int index);

/** Flatten surface to a mask
 *
 * @param sur Surface to convert
 */
void surface_flatten_to_mask(surface *sur, uint8_t value);

/**
 * Flatten each block of {block_size} colors by decrementing the index by {amount} in each block.
 * Start counting from {range_start} and stop at {range_end}. A block can be e.g. a color slide
 * of certain color.
 *
 * @param sur Surface to convert
 * @param range_start Palette range start index
 * @param range_end Palette range end index
 * @param block_size Palette color block size, e.g. 8
 * @param amount How much to decrement the index.
 */
void surface_compress_index_blocks(surface *sur, int range_start, int range_end, int block_size, int amount);

/**
 * Flatten a block of colors in palette by decrementing the index by {amount}. Start counting
 * from {range_start} and stop at {range_end}. If the resulting index after decrementing goes
 * below {range_start}, then continue decrementing from index {remap_to}. This virtually combines
 * two separate color blocks in the palette (e.g. color slides).
 *
 * @param sur Surface to convert
 * @param range_start Palette range start index
 * @param range_end Palette range end index
 * @param remap_to End index of a palette block to remap to
 * @param amount How much to decrement the index.
 */
void surface_compress_remap(surface *sur, int range_start, int range_end, int remap_to, int amount);

/**
 * Convert surface to grayscale using colors in palette from range-start to range-end (inclusive).
 * Conversion is done by luminosity.
 *
 * @param sur Surface to convert
 * @param pal Palette to use.
 * @param range_start First grey palette color
 * @param range_end Last gray palette color
 * @param ignore_below Leave indexes below this range alone
 */
void surface_convert_to_grayscale(surface *sur, const vga_palette *pal, int range_start, int range_end,
                                  int ignore_below);

/**
 * Convert surface's har colors to grayscale.
 * Remaps blocks of 16 colors in the range 0x00..0x5F to 0xD0..=0xDF.
 * Leaves other colors, and the transparent color alone.
 *
 * @param sur Surface to convert
 * @param brightness How bright to make the result (range 0x0.=0xF)
 */
void surface_convert_har_to_grayscale(surface *sur, uint8_t brightness);

/**
 * Write surface to a PNG file.
 *
 * @param sur Source surface
 * @param pal Palette to use (only applied if surface is paletted)
 * @param filename Target filename to write
 * @return True on success, false on any failure.
 */
bool surface_write_png(const surface *sur, const vga_palette *pal, const char *filename);

#endif // SURFACE_H
