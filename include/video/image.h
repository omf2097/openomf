#ifndef _IMAGE_H
#define _IMAGE_H

#include <shadowdive/rgba_image.h>
#include "video/color.h"

typedef struct image_t image;

struct image_t {
    unsigned int w,h;
    unsigned int stride;
    char *data;
};

int image_create(image *img, int w, int h);
int image_create_from_sd(image *img, sd_rgba_image *sdimg);
void image_free(image *img);

void image_clear(image *img, color c);
void image_line(image *img, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, color c);
void image_set_pixel(image *img, unsigned int x, unsigned int y, color c);
void image_rect(image *img, unsigned int x, unsigned int y, unsigned int w, unsigned int h, color c);
void image_filled_rect(image *img, unsigned int x, unsigned int y, unsigned int w, unsigned int h, color c); 

#endif // _IMAGE_H