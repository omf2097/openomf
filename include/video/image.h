#ifndef IMAGE_H
#define IMAGE_H

#include "formats/rgba_image.h"
#include "video/color.h"

typedef struct image_t {
    unsigned int w,h;
    char *data;
} image;

int image_create(image *img, int w, int h);
int image_create_from_sd(image *img, sd_rgba_image *sdimg);
void image_free(image *img);

void image_clear(image *img, color c);
void image_line(image *img, int x0, int y0, int x1, int y1, color c);
void image_set_pixel(image *img, int x, int y, color c);
void image_rect(image *img, int x, int y, int w, int h, color c);
void image_rect_bevel(image *img,
                      int x, int y,
                      int w, int h,
                      color ctop, color cright,
                      color cbottom, color left);
void image_filled_rect(image *img, int x, int y, int w, int h, color c);
int image_write_tga(image *img, const char *filename);

int image_write_png(image *img, const char *filename);

#endif // IMAGE_H
