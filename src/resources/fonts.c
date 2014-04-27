#include <shadowdive/shadowdive.h>

#include "utils/log.h"
#include "utils/vector.h"
#include "video/surface.h"
#include "resources/ids.h"
#include "resources/fonts.h"

font font_small;
font font_large;
static int fonts_loaded = 0;

void font_create(font *font) {
    font->size = FONT_UNDEFINED;
    vector_create(&font->surfaces, sizeof(surface*));
}

void font_free(font *font) {
    font->size = FONT_UNDEFINED;
    iterator it;
    vector_iter_begin(&font->surfaces, &it);
    surface **sur = NULL;
    while((sur = iter_next(&it)) != NULL) {
        surface_free(*sur);
        free(*sur);
    }
    vector_free(&font->surfaces);
}

int font_load(font *font, const char* filename, unsigned int size) {
    sd_rgba_image *img;
    sd_font *sdfont;
    int pixsize;
    surface *sur;

    // Find vertical size
    switch(size) {
        case FONT_BIG: pixsize = 8; break;
        case FONT_SMALL: pixsize = 6; break;
        default:
            return 1;
    }

    // Open font file
    sdfont = sd_font_create();
    if(sd_font_load(sdfont, filename, pixsize)) {
        sd_font_delete(sdfont);
        return 2;
    }

    // Load into textures
    img = sd_rgba_image_create(pixsize, pixsize);
    for(int i = 0; i < 224; i++) {
        sur = malloc(sizeof(surface));
        sd_font_decode(sdfont, img, i, 0xFF, 0xFF, 0xFF);
        surface_create_from_data(sur, SURFACE_TYPE_RGBA, img->w, img->h, img->data);
        vector_append(&font->surfaces, &sur);
    }

    // Set font info vars
    font->w = pixsize;
    font->h = pixsize;
    font->size = size;

    // Free resources
    sd_rgba_image_delete(img);
    sd_font_delete(sdfont);
    return 0;
}

int fonts_init() {
    font_create(&font_small);
    font_create(&font_large);
    char *filename = NULL;

    // Load small font
    filename = get_path_by_id(DAT_CHARSMAL);
    if(font_load(&font_small, filename, FONT_SMALL)) {
        PERROR("Unable to load font file '%s'!", filename);
        goto error_2;
    }
    INFO("Loaded font file '%s'", filename);
    free(filename);

    // Load big font
    filename = get_path_by_id(DAT_GRAPHCHR);
    if(font_load(&font_large, filename, FONT_BIG)) {
        PERROR("Unable to load font file '%s'!", filename);
        goto error_1;
    }
    INFO("Loaded font file '%s'", filename);
    free(filename);

    // All done.
    fonts_loaded = 1;
    return 0;

error_2:
    font_free(&font_small);
error_1:
    font_free(&font_large);
    free(filename);
    return 1;
}

void fonts_close() {
    if(fonts_loaded) {
        font_free(&font_small);
        font_free(&font_large);
        fonts_loaded = 0;
    }
}