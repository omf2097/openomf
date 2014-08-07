#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "shadowdive/rgba_image.h"
#include "shadowdive/error.h"

#define STRIDE 4

int sd_rgba_image_create(sd_rgba_image *img, unsigned int w, unsigned int h) {
    if(img == NULL) {
        return SD_INVALID_INPUT;
    }
    img->w = w;
    img->h = h;
    img->len = w * h * STRIDE;
    if((img->data = malloc(img->len)) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    memset(img->data, 0, img->len);
    return SD_SUCCESS;
}

int sd_rgba_image_copy(sd_rgba_image *dst, const sd_rgba_image *src) {
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    dst->w = src->w;
    dst->h = src->h;
    dst->len = src->len;
    if((dst->data = malloc(src->len)) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    memcpy(dst->data, src->data, src->len);
    return SD_SUCCESS;
}

int sd_rgba_image_to_ppm(const sd_rgba_image *img, const char *filename) {
    FILE *fd;
    int i;

    if((fd = fopen(filename, "wb")) == NULL) {
        return SD_FILE_OPEN_ERROR;
    }

    fprintf(fd, "P3\n");
    fprintf(fd, "# %s\n", filename);
    fprintf(fd, "%u %u\n", img->w, img->h);
    fprintf(fd, "255\n");
    i = 0;
    while(i < img->len) {
        for(int j = 0; j < 5; j++) {
            fprintf(fd, "%u %u %u ",
                (uint8_t)img->data[i],
                (uint8_t)img->data[i+1],
                (uint8_t)img->data[i+2]);
            i += 4;
        }
        fprintf(fd, "\n");
    }
    fclose(fd);
    return SD_SUCCESS;
}

void sd_rgba_image_free(sd_rgba_image *img) {
    if(img == NULL) return;
    free(img->data);
}
