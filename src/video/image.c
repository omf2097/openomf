#include "video/image.h"

#include <stdlib.h>
#include <memory.h>
#include <math.h>

int image_create(image *img, int w, int h) {
    img->data = malloc(w * h * 4);
    img->stride = w * 4;
    return 0;
}

void image_free(image *img) {
    free(img->data);
    img->data = NULL;
}

int image_create_from_sd(image *img, sd_rgba_image *sdimg) {
    image_create(img, sdimg->w, sdimg->h);
    memcpy(img->data, sdimg->data, sdimg->w * sdimg->h * 4);
    return 0;
}

void image_clear(image *img, color c) {
    for(int y = 0; y < img->h; y++) {
        for(int x = 0; x < img->w; x++) {
            image_set_pixel(img, x, y, c);
        }
    }
}

// Bresenham
void image_line(image *img, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, color c) {
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1; 
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;
 
    while(1) {
        image_set_pixel(img, x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void image_set_pixel(image *img, unsigned int x, unsigned int y, color c) {
    img->data[y * img->stride + 4 * x + 0] = c.r;
    img->data[y * img->stride + 4 * x + 1] = c.g;
    img->data[y * img->stride + 4 * x + 2] = c.b;
    img->data[y * img->stride + 4 * x + 3] = c.a;
}

void image_rect(image *img, unsigned int x, unsigned int y, unsigned int w, unsigned int h, color c) {
    image_line(img, x, y, x+w, y, c);
    image_line(img, x, y+h, x+w, y+h, c);
    image_line(img, x+w, y, x+w, y+h, c);
    image_line(img, x, y, x, y+h, c);
}

void image_filled_rect(image *img, unsigned int x, unsigned int y, unsigned int w, unsigned int h, color c) {

}
 