#ifndef SCREEN_PALETTE
#define SCREEN_PALETTE

#include <stdint.h>

typedef struct {
    uint8_t data[256][3];
    unsigned int version;
} screen_palette;

#endif // SCREEN_PALETTE
