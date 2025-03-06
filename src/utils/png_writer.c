#include "utils/png_writer.h"
#include "utils/log.h"

#ifdef PNG_FOUND
#include "utils/crash.h"
#include <assert.h>
#include <png.h>

static void abort_png(png_structp png, const char *err) {
    crash(err);
}

bool write_rgb_png(const char *filename, int w, int h, const unsigned char *data, bool has_alpha, bool flip) {
    assert(filename != NULL);
    assert(data != NULL);

    FILE *handle;
    png_structp png_ptr;
    png_infop info_ptr;

    if((handle = fopen(filename, "wb")) == NULL) {
        log_error("Unable to write PNG file: Could not open file for writing");
        return false;
    }
    if(!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, &abort_png, NULL))) {
        crash("Unable to allocate libpng write struct: Out of memory!");
    }
    if(!(info_ptr = png_create_info_struct(png_ptr))) {
        crash("Unable to allocate libpng info struct: Out of memory!");
    }

    png_init_io(png_ptr, handle);
    png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    // Write rows; take into account write order (might be flipped)
    int bytes = (has_alpha ? 4 : 3);
    int pos;
    for(int y = 0; y < h; y++) {
        pos = flip ? (h - y - 1) : y;
        png_write_row(png_ptr, data + pos * w * bytes);
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(handle);
    return true;
}

bool write_paletted_png(const char *filename, int w, int h, const vga_palette *pal, const unsigned char *data) {
    assert(filename != NULL);
    assert(data != NULL);
    assert(w * h > 0);

    png_image out;
    memset(&out, 0, sizeof(out));
    out.version = PNG_IMAGE_VERSION;
    out.opaque = NULL;
    out.width = w;
    out.height = h;
    out.flags = 0;
    out.format = PNG_FORMAT_RGB_COLORMAP;
    out.colormap_entries = 256;
    png_image_write_to_file(&out, filename, 0, data, w, pal->colors);

    if(PNG_IMAGE_FAILED(out)) {
        log_error("Unable to write PNG file: %s", out.message);
        return false;
    }
    return true;
}

#else // PNG_FOUND

bool png_write_rgb(const char *filename, int w, int h, const unsigned char *data, bool has_alpha, bool flip) {
    PERROR("PNG writing is not supported in current build!");
    return false;
}

bool png_write_paletted(const char *filename, int w, int h, const vga_palette *pal, const unsigned char *data) {
    PERROR("PNG writing is not supported in current build!");
    return false;
}

#endif // PNG_FOUND
