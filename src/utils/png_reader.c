#include "utils/png_reader.h"
#include "utils/log.h"

#ifdef PNG_FOUND
#include <assert.h>
#include <png.h>
#include <stdint.h>

#include "utils/allocator.h"

bool read_paletted_png(const char *filename, unsigned char *dst) {
    assert(filename != NULL);
    assert(dst != NULL);
    bool ret = false;
    FILE *handle = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep *row_pointers = NULL;
    uint8_t sig[8];

    if((handle = fopen(filename, "rb")) == NULL) {
        goto error_0;
    }
    if(fread(sig, 1, 8, handle) != 8) {
        goto error_1;
    }
    if(!png_check_sig(sig, 8)) {
        goto error_1;
    }
    if(!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
        goto error_1;
    }
    if(!(info_ptr = png_create_info_struct(png_ptr))) {
        goto error_1;
    }
    if(setjmp(png_jmpbuf(png_ptr))) {
        goto error_2;
    }

    png_init_io(png_ptr, handle);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    int w = png_get_image_width(png_ptr, info_ptr);
    int h = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // We need 320x200 palette image.
    if(w > 320 || w < 1 || h > 200 || h < 1 || color_type != PNG_COLOR_TYPE_PALETTE || bit_depth != 8) {
        goto error_2;
    }

    // Allocate memory for the data
    row_pointers = omf_malloc(h * sizeof(png_bytep));
    for(int y = 0; y < h; y++) {
        row_pointers[y] = omf_malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        goto error_3;
    }

    png_read_image(png_ptr, row_pointers);

    // Convert from PNG source to destination buffer
    for(int y = 0; y < h; y++) {
        png_byte *row = row_pointers[y];
        for(int x = 0; x < w; x++) {
            dst[w * y + x] = row[x];
        }
    }
    ret = true;

    // Free up everything
error_3:
    for(int y = 0; y < h; y++) {
        omf_free(row_pointers[y]);
    }
    omf_free(row_pointers);
error_2:
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
error_1:
    fclose(handle);
error_0:
    return ret;
}

#else // PNG_FOUND

bool png_read_paletted(const char *filename, unsigned char *dst) {
    PERROR("PNG reading is not supported in current build!");
    return false;
}

#endif // PNG_FOUND
