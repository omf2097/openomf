#include "utils/png_reader.h"
#include "utils/log.h"

#ifdef PNG_FOUND
#include <assert.h>
#include <png.h>
#include <stdint.h>

#include "utils/allocator.h"

#define SIGNATURE_SIZE 8

static void abort_png(png_structp png, const char *err) {
    log_error("libpng error: %s", err);
    abort();
}

static FILE *open_and_check(const char *filename) {
    FILE *handle = fopen(filename, "rb");
    if(handle == NULL) {
        return NULL;
    }
    uint8_t sig[SIGNATURE_SIZE];
    if(fread(sig, 1, SIGNATURE_SIZE, handle) != SIGNATURE_SIZE) {
        goto error;
    }
    if(!png_check_sig(sig, 8)) {
        goto error;
    }
    return handle;

error:
    fclose(handle);
    return NULL;
}

static void read_png_data(unsigned char *dst, png_structp png_ptr, png_infop info_ptr, int w, int h) {
    png_bytep *row_pointers = png_malloc(png_ptr, h * png_get_rowbytes(png_ptr, info_ptr));
    png_read_image(png_ptr, row_pointers);
    for(int y = 0; y < h; y++) {
        png_byte *row = row_pointers[y];
        for(int x = 0; x < w; x++) {
            dst[w * y + x] = row[x];
        }
    }
    png_free(png_ptr, row_pointers);
}

bool read_paletted_png(const char *filename, unsigned char *dst) {
    assert(filename != NULL);
    assert(dst != NULL);

    FILE *handle = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    if((handle = open_and_check(filename)) == NULL) {
        log_error("Unable to read PNG file: Could not open file for reading");
        return false;
    }
    if(!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, &abort_png, NULL))) {
        log_error("Unable to allocate libpng read struct: Out of memory!");
        abort();
    }
    if(!(info_ptr = png_create_info_struct(png_ptr))) {
        log_error("Unable to allocate libpng info struct: Out of memory!");
        abort();
    }

    png_init_io(png_ptr, handle);
    png_set_sig_bytes(png_ptr, SIGNATURE_SIZE);
    png_read_info(png_ptr, info_ptr);
    int w = png_get_image_width(png_ptr, info_ptr);
    int h = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    bool valid = (w == 320 && h == 200 && color_type == PNG_COLOR_TYPE_PALETTE && bit_depth == 8);
    if(valid) {
        read_png_data(dst, png_ptr, info_ptr, w, h);
    } else {
        log_error("PNG must be paletted 320x200 image");
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(handle);
    return valid;
}

#else // PNG_FOUND

bool png_read_paletted(const char *filename, unsigned char *dst) {
    PERROR("PNG reading is not supported in current build!");
    return false;
}

#endif // PNG_FOUND
