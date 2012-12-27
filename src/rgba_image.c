#include "rgba_image.h"
#include <stdlib.h>
#include <string.h>

sd_rgba_image* sd_rgba_image_create(unsigned int w, unsigned int h) {
    sd_rgba_image *img = (sd_rgba_image*)malloc(sizeof(sd_rgba_image));
    img->data = (char*)malloc(w * h * 4);
    img->w = w;
    img->h = h;
    memset(img->data, w*h*4, 0);
    return img;
}

void sd_rgba_image_delete(sd_rgba_image *img) {
    free(img->data);
    free(img);
}
