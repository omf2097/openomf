#ifndef PNG_READER_H
#define PNG_READER_H

#include "utils/path.h"
#include "video/vga_palette.h"
#include <stdbool.h>

bool read_paletted_png(const path *filename, unsigned char *dst);
bool read_paletted_png_from_memory(const unsigned char *buf, size_t len, unsigned char *dst, int *w, int *h,
                                   bool allow_transparency, vga_palette *pal);

#endif // PNG_READER_H
