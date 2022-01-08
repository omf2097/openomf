#include "formats/fonts.h"
#include "formats/error.h"
#include "resources/fonts.h"
#include "resources/ids.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "video/surface.h"

font font_small;
font font_large;
static int fonts_loaded = 0;

void font_create(font *f) {
    memset(f, 0, sizeof(font));
    vector_create(&f->surfaces, sizeof(surface *));
}

void font_free(font *font) {
    iterator it;
    vector_iter_begin(&font->surfaces, &it);
    surface **sur = NULL;
    while ((sur = iter_next(&it)) != NULL) {
        surface_free(*sur);
        omf_free(*sur);
    }
    vector_free(&font->surfaces);
}

int font_load(font *font, const char *filename, unsigned int size) {
    sd_rgba_image img;
    sd_font sdfont;
    int pixsize;
    surface *sur;

    // Find vertical size
    switch (size) {
    case FONT_BIG:
        pixsize = 8;
        break;
    case FONT_SMALL:
        pixsize = 6;
        break;
    default:
        return 1;
    }

    // Open font file
    if (sd_font_create(&sdfont) != SD_SUCCESS) {
        return 1;
    }
    if (sd_font_load(&sdfont, filename, pixsize)) {
        sd_font_free(&sdfont);
        return 2;
    }

    // Load into textures
    sd_rgba_image_create(&img, pixsize, pixsize);
    for (int i = 0; i < 224; i++) {
        sur = omf_calloc(1, sizeof(surface));
        sd_font_decode(&sdfont, &img, i, 0xFF, 0xFF, 0xFF);
        surface_create_from_data(sur, SURFACE_TYPE_RGBA, img.w, img.h, img.data);
        vector_append(&font->surfaces, &sur);
    }

    // Set font info vars
    font->w = pixsize;
    font->h = pixsize;
    font->size = size;

    // Free resources
    sd_rgba_image_free(&img);
    sd_font_free(&sdfont);
    return 0;
}

int fonts_init() {
    font_create(&font_small);
    font_create(&font_large);
    const char *filename = NULL;

    // Load small font
    filename = pm_get_resource_path(DAT_CHARSMAL);
    if (font_load(&font_small, filename, FONT_SMALL)) {
        PERROR("Unable to load font file '%s'!", filename);
        goto error_2;
    }
    INFO("Loaded font file '%s'", filename);

    // Load big font
    filename = pm_get_resource_path(DAT_GRAPHCHR);
    if (font_load(&font_large, filename, FONT_BIG)) {
        PERROR("Unable to load font file '%s'!", filename);
        goto error_1;
    }
    INFO("Loaded font file '%s'", filename);

    // All done.
    fonts_loaded = 1;
    return 0;

error_2:
    font_free(&font_small);
error_1:
    font_free(&font_large);
    return 1;
}

void fonts_close() {
    if (fonts_loaded) {
        font_free(&font_small);
        font_free(&font_large);
        fonts_loaded = 0;
    }
}