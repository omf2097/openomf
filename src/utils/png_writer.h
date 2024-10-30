#ifndef PNG_WRITER_H
#define PNG_WRITER_H

#include "video/vga_palette.h"
#include <stdbool.h>

bool png_write_rgb(const char *filename, int w, int h, const unsigned char *data, bool has_alpha, bool flip);
bool png_write_paletted(const char *filename, int w, int h, const vga_palette *pal, const unsigned char *data);

#endif // PNG_WRITER_H
