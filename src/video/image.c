#include <math.h>
#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "video/image.h"

#define CHECK_COORD_BOUNDS(n, x) n = (n >= x ? (x - 1) : (n < 0 ? 0 : n))

int image_create(image *img, int w, int h) {
    img->data = omf_calloc(1, w * h);
    img->w = w;
    img->h = h;
    return 0;
}

void image_free(image *img) {
    omf_free(img->data);
    img->w = 0;
    img->h = 0;
}

void image_clear(image *img, uint8_t c) {
    memset(img->data, c, img->w * img->h);
}

// Bresenham
void image_line(image *img, int x0, int y0, int x1, int y1, uint8_t c) {
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    while(1) {
        image_set_pixel(img, x0, y0, c);
        if(x0 == x1 && y0 == y1)
            break;
        e2 = err;
        if(e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if(e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

inline void image_set_pixel(image *img, int x, int y, uint8_t c) {
    CHECK_COORD_BOUNDS(x, img->w);
    CHECK_COORD_BOUNDS(y, img->h);
    img->data[y * img->w + x] = c;
}

void image_rect(image *img, int x, int y, int w, int h, uint8_t c) {
    image_line(img, x, y, x + w, y, c);
    image_line(img, x, y + h, x + w, y + h, c);
    image_line(img, x + w, y, x + w, y + h, c);
    image_line(img, x, y, x, y + h, c);
}

void image_rect_bevel(image *img, int x, int y, int w, int h, uint8_t ctop, uint8_t cright, uint8_t cbottom,
                      uint8_t cleft) {
    image_line(img, x, y, x + w, y, ctop);
    image_line(img, x, y + h, x + w, y + h, cbottom);
    image_line(img, x + w, y, x + w, y + h, cright);
    image_line(img, x, y, x, y + h, cleft);
}
