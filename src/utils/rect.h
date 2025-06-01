#ifndef RECT_H
#define RECT_H

#include <assert.h>
#include <stdint.h>

typedef struct rect16 {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
} rect16;

static_assert(8 == sizeof(rect16), "rect16 should pack into 8 bytes");

typedef struct rect32 {
    int32_t x;
    int32_t y;
    uint32_t w;
    uint32_t h;
} rect32;

static_assert(16 == sizeof(rect32), "rect32 should pack into 16 bytes");

#endif // RECT_H
