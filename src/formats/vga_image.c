#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/vga_image.h"
#include "utils/allocator.h"

int sd_vga_image_create(sd_vga_image *img, unsigned int w, unsigned int h) {
    if(img == NULL) {
        return SD_INVALID_INPUT;
    }
    img->w = w;
    img->h = h;
    img->len = w * h;
    img->data = omf_calloc(1, w * h);
    return SD_SUCCESS;
}

int sd_vga_image_copy(sd_vga_image *dst, const sd_vga_image *src) {
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

void sd_vga_image_free(sd_vga_image *img) {
    if(img == NULL) {
        return;
    }
    omf_free(img->data);
}

int sd_vga_image_decode(sd_rgba_image *dst, const sd_vga_image *src, const palette *pal, int remapping) {
    int ret;
    if(dst == NULL || src == NULL || pal == NULL) {
        return SD_INVALID_INPUT;
    }
    if((ret = sd_rgba_image_create(dst, src->w, src->h)) != SD_SUCCESS) {
        return ret;
    }
    int pos = 0;
    for(int y = src->h - 1; y >= 0; y--) {
        for(int x = 0; x < src->w; x++) {
            uint8_t b = src->data[y * src->w + x];
            pos = ((y * src->w) + x) * 4;
            if(remapping > -1) {
                dst->data[pos + 0] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][0];
                dst->data[pos + 1] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][1];
                dst->data[pos + 2] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][2];
                dst->data[pos + 3] = (uint8_t)255;
            } else {
                dst->data[pos + 0] = (uint8_t)pal->data[b][0];
                dst->data[pos + 1] = (uint8_t)pal->data[b][1];
                dst->data[pos + 2] = (uint8_t)pal->data[b][2];
                dst->data[pos + 3] = (uint8_t)255;
            }
        }
    }
    return SD_SUCCESS;
}

int sd_vga_image_from_png(sd_vga_image *img, const char *filename) {
    png_structp png_ptr;
    png_infop info_ptr;
    int ret = SD_SUCCESS;
    int got = 0;
    png_bytep *row_pointers;

    if(img == NULL || filename == NULL) {
        ret = SD_INVALID_INPUT;
        goto error_0;
    }

    FILE *handle = fopen(filename, "rb");
    if(handle == NULL) {
        ret = SD_FILE_OPEN_ERROR;
        goto error_0;
    }

    uint8_t sig[8];
    got = fread(sig, 1, 8, handle);
    if(got != 8 || !png_check_sig(sig, 8)) {
        ret = SD_FILE_INVALID_TYPE;
        goto error_1;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
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
        ret = SD_FILE_INVALID_TYPE;
        goto error_2;
    }

    png_init_io(png_ptr, handle);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    int w = png_get_image_width(png_ptr, info_ptr);
    int h = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if(w > 320 || w < 1 || h > 200 || h < 1) {
        ret = SD_FILE_INVALID_TYPE;
        goto error_2;
    }

    // We need paletted images.
    if(color_type != PNG_COLOR_TYPE_PALETTE || bit_depth != 8) {
        ret = SD_FILE_INVALID_TYPE;
        goto error_2;
    }

    // Allocate memory for the data
    row_pointers = omf_calloc(h, sizeof(png_bytep));
    for(int y = 0; y < h; y++) {
        row_pointers[y] = omf_calloc(1, png_get_rowbytes(png_ptr, info_ptr));
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        ret = SD_FILE_INVALID_TYPE;
        goto error_3;
    }

    png_read_image(png_ptr, row_pointers);

    // Convert
    if(sd_vga_image_create(img, w, h) != SD_SUCCESS) {
        ret = SD_OUT_OF_MEMORY;
        goto error_3;
    }
    for(int y = 0; y < h; y++) {
        png_byte *row = row_pointers[y];
        for(int x = 0; x < w; x++) {
            img->data[w * y + x] = row[x];
        }
    }

    // Free up everything
error_3:
    for(int y = 0; y < h; y++) {
        omf_free(row_pointers[y]);
    }
    omf_free(row_pointers);
error_2:
    png_destroy_read_struct(&png_ptr, NULL, NULL);
error_1:
    fclose(handle);
error_0:
    return ret;
}

int sd_vga_image_to_png(const sd_vga_image *img, const palette *pal, const char *filename) {
    if(img == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;
    int ret = SD_SUCCESS;
    char *rows[img->h];

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

    // Write header. Paletted, 8 bits per pixel
    png_set_IHDR(png_ptr, info_ptr, img->w, img->h, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    palette = png_malloc(png_ptr, 256 * sizeof(png_color));
    for(int i = 0; i < 256; i++) {
        palette[i].red = pal->data[i][0];
        palette[i].green = pal->data[i][1];
        palette[i].blue = pal->data[i][2];
    }
    png_set_PLTE(png_ptr, info_ptr, palette, 256);

    if(setjmp(png_jmpbuf(png_ptr))) {
        ret = SD_OUT_OF_MEMORY;
        goto error_3;
    }

    for(int y = 0; y < img->h; y++) {
        rows[y] = img->data + y * img->w;
    }

    // Write data
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, (void *)rows);
    png_write_end(png_ptr, NULL);

    // Free everything
error_3:
    png_free(png_ptr, palette);
error_2:
    png_destroy_write_struct(&png_ptr, NULL);
error_1:
    fclose(handle);
error_0:
    return ret;
}
