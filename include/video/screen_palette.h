#ifndef _SCREEN_PALETTE
#define _SCREEN_PALETTE

#include <stdint.h>

typedef struct {
    uint8_t data[256][3];
    unsigned int version;
} screen_palette;

#endif // _SCREEN_PALETTE