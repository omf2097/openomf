#ifndef PNG_READER_H
#define PNG_READER_H

#include "video/vga_palette.h"
#include <stdbool.h>

bool png_read_paletted(const char *filename, unsigned char *dst);

#endif // PNG_READER_H
