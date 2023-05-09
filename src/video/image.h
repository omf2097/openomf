#ifndef IMAGE_H
#define IMAGE_H

#include "formats/rgba_image.h"
#include "video/color.h"

typedef struct image_t {
    unsigned int w, h;
    char *data;
} image;

int image_create(image *img, int w, int h);
void image_free(image *img);

void image_clear(image *img, uint8_t c);
void image_line(image *img, int x0, int y0, int x1, int y1, uint8_t c);
void image_set_pixel(image *img, int x, int y, uint8_t c);
void image_rect(image *img, int x, int y, int w, int h, uint8_t c);
void image_rect_bevel(image *img, int x, int y, int w, int h, uint8_t ctop, uint8_t cright, uint8_t cbottom,
                      uint8_t left);
void image_filled_rect(image *img, int x, int y, int w, int h, uint8_t c);

int image_write_png(image *img, const char *filename);

#endif // IMAGE_H
