#ifndef PNG_WRITER_H
#define PNG_WRITER_H

#include "utils/path.h"
#include "video/vga_palette.h"
#include <stdbool.h>

bool write_rgb_png(const path *filename, int w, int h, const unsigned char *data, bool has_alpha, bool flip);
bool write_paletted_png(const path *filename, int w, int h, const vga_palette *pal, const unsigned char *data);

#endif // PNG_WRITER_H
