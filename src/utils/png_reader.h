/**
 * @file png_reader.h
 * @brief PNG image file reading.
 * @details Functions for reading paletted PNG image data.
 */

#ifndef PNG_READER_H
#define PNG_READER_H

#include "utils/path.h"
#include <stdbool.h>

/**
 * @brief Read a paletted PNG file into a buffer.
 * @details The PNG must be an indexed (paletted) image. The destination buffer
 *          must be pre-allocated with sufficient size (width * height bytes).
 * @param filename Path to the PNG file to read
 * @param dst Destination buffer for pixel data (palette indices)
 * @return true on success, false on failure
 */
bool read_paletted_png(const path *filename, unsigned char *dst);

#endif // PNG_READER_H
