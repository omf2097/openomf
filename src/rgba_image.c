#include "shadowdive/rgba_image.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

sd_rgba_image* sd_rgba_image_create(unsigned int w, unsigned int h) {
    sd_rgba_image *img = (sd_rgba_image*)malloc(sizeof(sd_rgba_image));
    img->data = (char*)malloc(w * h * 4);
    img->w = w;
    img->h = h;
    memset(img->data, 0, w*h*4);
    return img;
}

void sd_rgba_image_to_ppm(sd_rgba_image *img, const char *filename) {
    FILE *fd = fopen(filename, "wb");
    fprintf(fd, "P3\n");
    fprintf(fd, "# %s\n", filename);
    fprintf(fd, "%u %u\n", img->w, img->h);
    fprintf(fd, "255\n");
    int len = img->w * img->h * 4;
    int i = 0;
    while(i < len) {
        for(int j = 0; j < 5; j++) {
            fprintf(fd, "%u %u %u ", (uint8_t)img->data[i], (uint8_t)img->data[i+1], (uint8_t)img->data[i+2]);
            i+=4;
        }
        fprintf(fd, "\n");
    }
    fclose(fd);
}

void sd_rgba_image_delete(sd_rgba_image *img) {
    free(img->data);
    free(img);
}
