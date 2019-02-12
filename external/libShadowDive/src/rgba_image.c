#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "shadowdive/rgba_image.h"
#include "shadowdive/error.h"

#ifdef USE_PNG
    #include <png.h>
#endif

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

int sd_rgba_image_blit(sd_rgba_image *dst, const sd_rgba_image *src, int x, int y) {
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }
    if(x < 0 || y < 0) {
        return SD_INVALID_INPUT;
    }

    int rdp,rsp;
    for(int py = 0; py < src->h; py++) {
        for(int px = 0; px < src->w; px++) {
            rdp = (py + y) * dst->w + (px + x);
            rsp = py * src->w + px;
            dst->data[rdp*4 + 0] = src->data[rsp*4 + 0];
            dst->data[rdp*4 + 1] = src->data[rsp*4 + 1];
            dst->data[rdp*4 + 2] = src->data[rsp*4 + 2];
            dst->data[rdp*4 + 3] = src->data[rsp*4 + 3];
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
#ifdef USE_PNG
    png_structp png_ptr;
    png_infop info_ptr;
    int ret = SD_SUCCESS;

    if(img == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    char *rows[img->h];
    for(int y = 0; y < img->h; y++) {
        rows[y] = img->data + (y * img->w * 4);
    }

    FILE *handle = fopen(filename, "wb");
    if(handle == NULL) {
        ret = SD_FILE_OPEN_ERROR;
        goto error_0;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        ret = SD_OUT_OF_MEMORY;
        goto error_1;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        ret = SD_OUT_OF_MEMORY;
        goto error_2;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        ret = SD_OUT_OF_MEMORY;
        goto error_2;
    }

    png_init_io(png_ptr, handle);

    // Write header. RGBA, 8 bits per pixel
    png_set_IHDR(png_ptr,
                 info_ptr,
                 img->w,
                 img->h,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    if(setjmp(png_jmpbuf(png_ptr))) {
        ret = SD_OUT_OF_MEMORY;
        goto error_2;
    }

    // Write data
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, (void*)rows);
    png_write_end(png_ptr, NULL);

    // Free everything
error_2:
    png_destroy_write_struct(&png_ptr, NULL);
error_1:
    fclose(handle);
error_0:
    return ret;
#else
    return SD_FORMAT_NOT_SUPPORTED;
#endif
}

void sd_rgba_image_free(sd_rgba_image *img) {
    if(img == NULL)
        return;
    free(img->data);
}
