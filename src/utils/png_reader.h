#ifndef PNG_READER_H
#define PNG_READER_H

#include "video/vga_palette.h"
#include <stdbool.h>

bool read_paletted_png(const char *filename, unsigned char *dst);

#endif // PNG_READER_H
