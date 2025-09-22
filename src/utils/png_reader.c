#include "utils/png_reader.h"
#include "utils/log.h"

#ifdef PNG_FOUND
#include <assert.h>
#include <png.h>
#include <stdint.h>

static_assert(PNG_LIBPNG_VER >= 10600, "libpng version should be 1.6.0 or later");

#include "utils/allocator.h"
#include "utils/crash.h"

#define SIGNATURE_SIZE 8

static void abort_png(png_structp png, const char *err) {
    crash(err);
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
    png_uint_32 row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep *row_pointers = png_malloc(png_ptr, h * row_bytes);
    log_info("allocated %dx%d bytes for row pointers", h, row_bytes);

    for(int y = 0; y < h; y++) {
        row_pointers[y] = dst + (y * row_bytes);
    }

    png_read_image(png_ptr, row_pointers);
    png_free(png_ptr, row_pointers);
}

bool read_paletted_png(const path *filename, unsigned char *dst) {
    assert(filename != NULL);
    assert(dst != NULL);

    FILE *handle = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    if((handle = open_and_check(path_c(filename))) == NULL) {
        log_error("Unable to read PNG file: Could not open file for reading");
        return false;
    }
    if(!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, &abort_png, NULL))) {
        crash("Unable to allocate libpng read struct: Out of memory!");
    }
    if(!(info_ptr = png_create_info_struct(png_ptr))) {
        crash("Unable to allocate libpng info struct: Out of memory!");
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

typedef struct {
    const unsigned char *buffer;
    size_t size;
    size_t offset;
} memory_reader_state;

static void read_memory_data(png_structp png_ptr, png_bytep out, png_size_t length) {
    memory_reader_state *state = (memory_reader_state *)png_get_io_ptr(png_ptr);
    if(state->offset + length > state->size) {
        png_error(png_ptr, "Read beyond end of buffer");
    }
    memcpy(out, state->buffer + state->offset, length);
    state->offset += length;
}

bool read_paletted_png_from_memory(const unsigned char *buffer, size_t size, unsigned char *dst, int *w, int *h,
                                   bool allow_transparency) {
    assert(buffer != NULL);

    // Check signature
    if(size < SIGNATURE_SIZE || !png_check_sig((png_bytep)buffer, SIGNATURE_SIZE)) {
        log_error("Invalid PNG signature in memory buffer");
        return false;
    }

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    if(!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, &abort_png, NULL))) {
        crash("Unable to allocate libpng read struct: Out of memory!");
    }
    if(!(info_ptr = png_create_info_struct(png_ptr))) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        crash("Unable to allocate libpng info struct: Out of memory!");
    }

    // Set up memory reading
    memory_reader_state state = {buffer + SIGNATURE_SIZE, size - SIGNATURE_SIZE, 0};
    png_set_read_fn(png_ptr, &state, read_memory_data);
    png_set_sig_bytes(png_ptr, SIGNATURE_SIZE);

    png_read_info(png_ptr, info_ptr);

    png_bytep trans = NULL;
    int num_trans = 0;
    png_color_16p trans_values = NULL;

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values);
    }

    *w = png_get_image_width(png_ptr, info_ptr);
    *h = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if(!allow_transparency && trans) {
        log_error("PNG contained transparent pixels");
        return false;
    }

    bool valid = color_type == PNG_COLOR_TYPE_PALETTE && bit_depth == 8;
    if(valid) {
        if(dst) {
            // if DST is null, we're probably trying to get the size
            read_png_data(dst, png_ptr, info_ptr, *w, *h);

            // remap all transparent pixels to palette index 0
            if(allow_transparency && trans) {
                for(int y = 0; y < *h; y++) {
                    for(int x = 0; x < *w; x++) {
                        int idx = (*w) * y + x;
                        png_byte pixel = dst[idx];
                        if(pixel < num_trans && trans[pixel] == 0) {
                            dst[idx] = 0;
                        }
                    }
                }
            }
        }
    } else {
        log_error("PNG must be paletted image");
    }

    log_info("png is %dx%d", *w, *h);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return valid;
}

#else // PNG_FOUND

bool read_paletted_png(const path *filename, unsigned char *dst) {
    PERROR("PNG reading is not supported in current build!");
    return false;
}

bool read_paletted_png_from_memory(const char *filename, unsigned char *dst, int *w, int *h) {
    PERROR("PNG reading is not supported in current build!");
    return false;
}

#endif // PNG_FOUND
