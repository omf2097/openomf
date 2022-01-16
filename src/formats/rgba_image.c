#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/rgba_image.h"
#include "utils/allocator.h"
#include "utils/log.h"

#define STRIDE 4

int sd_rgba_image_create(sd_rgba_image *img, unsigned int w, unsigned int h) {
    if(img == NULL) {
        return SD_INVALID_INPUT;
    }
    img->w = w;
    img->h = h;
    img->len = w * h * STRIDE;
    img->data = omf_calloc(img->len, 1);
    return SD_SUCCESS;
}

int sd_rgba_image_copy(sd_rgba_image *dst, const sd_rgba_image *src) {
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    dst->w = src->w;
    dst->h = src->h;
    dst->len = src->len;
    dst->data = omf_calloc(src->len, 1);
    memcpy(dst->data, src->data, src->len);
    return SD_SUCCESS;
}

int sd_rgba_image_blit(sd_rgba_image *dst, const sd_rgba_image *src, int x, int y) {
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }
    if(x < 0 || y < 0) {
        return SD_INVALID_INPUT;
    }

    int rdp, rsp;
    for(int py = 0; py < src->h; py++) {
        for(int px = 0; px < src->w; px++) {
            rdp = (py + y) * dst->w + (px + x);
            rsp = py * src->w + px;
            dst->data[rdp * 4 + 0] = src->data[rsp * 4 + 0];
            dst->data[rdp * 4 + 1] = src->data[rsp * 4 + 1];
            dst->data[rdp * 4 + 2] = src->data[rsp * 4 + 2];
            dst->data[rdp * 4 + 3] = src->data[rsp * 4 + 3];
        }
    }

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
            fprintf(fd, "%u %u %u ", (uint8_t)img->data[i], (uint8_t)img->data[i + 1], (uint8_t)img->data[i + 2]);
            i += 4;
        }
        fprintf(fd, "\n");
    }
    fclose(fd);
    return SD_SUCCESS;
}

int sd_rgba_image_clear(sd_rgba_image *img, char r, char g, char b, char a) {
    if(img == NULL) {
        return SD_INVALID_INPUT;
    }
    for(int y = 0; y < img->h; y++) {
        for(int x = 0; x < img->w; x++) {
            img->data[(y * img->w + x) * 4 + 0] = r;
            img->data[(y * img->w + x) * 4 + 1] = g;
            img->data[(y * img->w + x) * 4 + 2] = b;
            img->data[(y * img->w + x) * 4 + 3] = a;
        }
    }
    return SD_SUCCESS;
}

int sd_rgba_image_to_png(const sd_rgba_image *img, const char *filename) {
    if(img == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    png_image out;
    memset(&out, 0, sizeof(out));
    out.version = PNG_IMAGE_VERSION;
    out.opaque = NULL;
    out.width = img->w;
    out.height = img->h;
    out.format = PNG_FORMAT_RGBA;
    out.flags = 0;
    out.colormap_entries = 0;

    png_image_write_to_file(&out, filename, 0, img->data, img->w * 4, NULL);

    if(PNG_IMAGE_FAILED(out)) {
        PERROR("Unable to write PNG file: %s", out.message);
        return SD_FILE_WRITE_ERROR;
    }
    return SD_SUCCESS;
}

void sd_rgba_image_free(sd_rgba_image *img) {
    if(img == NULL) {
        return;
    }
    omf_free(img->data);
}
