#ifndef SCREEN_PALETTE_H
#define SCREEN_PALETTE_H

#include <stdint.h>

typedef struct {
    uint8_t data[256][3];
    unsigned int version;
} screen_palette;

#endif // SCREEN_PALETTE_H
