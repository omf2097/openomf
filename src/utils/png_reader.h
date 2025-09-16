#ifndef PNG_READER_H
#define PNG_READER_H

#include "utils/path.h"
#include <stdbool.h>

bool read_paletted_png(const path *filename, unsigned char *dst);

#endif // PNG_READER_H
