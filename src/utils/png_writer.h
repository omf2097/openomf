/**
 * @file png_writer.h
 * @brief PNG image file writing.
 * @details Functions for writing image data to PNG files.
 *          Supports both RGB/RGBA and paletted image formats.
 */

#ifndef PNG_WRITER_H
#define PNG_WRITER_H

#include "utils/path.h"
#include "video/vga_palette.h"
#include <stdbool.h>

/**
 * @brief Write RGB or RGBA image data to a PNG file.
 * @param filename Path to the output PNG file
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param data Pixel data (RGB or RGBA depending on has_alpha)
 * @param has_alpha If true, data contains RGBA pixels (4 bytes each);
 *                  if false, data contains RGB pixels (3 bytes each)
 * @param flip If true, flip the image vertically during write
 * @return true on success, false on failure
 */
bool write_rgb_png(const path *filename, int w, int h, const unsigned char *data, bool has_alpha, bool flip);

/**
 * @brief Write paletted image data to a PNG file.
 * @param filename Path to the output PNG file
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param pal VGA palette to use for the image
 * @param data Pixel data as palette indices (1 byte per pixel)
 * @return true on success, false on failure
 */
bool write_paletted_png(const path *filename, int w, int h, const vga_palette *pal, const unsigned char *data);

#endif // PNG_WRITER_H
