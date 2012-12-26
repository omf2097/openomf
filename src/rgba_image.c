#include "rgba_image.h"
#include <stdlib.h>
#include <string.h>

rgba_image* sd_rgba_image_create(unsigned int w, unsigned int h) {
    rgba_image *img = (rgba_image*)malloc(sizeof(rgba_image));
    img->data = (char*)malloc(w * h * 4);
    img->w = w;
    img->h = h;
    memset(img->data, w*h*4, 0);
    return img;
}

void sd_rgba_image_delete(rgba_image *img) {
    free(img->data);
    free(img);
}
