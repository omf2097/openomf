#include "formats/fonts.h"
#include "formats/error.h"
#include "formats/pcx.h"
#include "resources/fonts.h"
#include "resources/ids.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "video/surface.h"

font font_small;
font font_large;
font font_net1;
font font_net2;
static int fonts_loaded = 0;

void font_create(font *f) {
    memset(f, 0, sizeof(font));
    vector_create(&f->surfaces, sizeof(surface *));
}

void font_free(font *font) {
    iterator it;
    vector_iter_begin(&font->surfaces, &it);
    surface **sur = NULL;
    while((sur = iter_next(&it)) != NULL) {
        surface_free(*sur);
        omf_free(*sur);
    }
    vector_free(&font->surfaces);
}

int font_load(font *font, const char *filename, unsigned int size) {
    sd_vga_image img;
    sd_font sdfont;
    int pixsize;
    surface *sur;

    // Find vertical size
    switch(size) {
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
    if(sd_font_create(&sdfont) != SD_SUCCESS) {
        return 1;
    }
    if(sd_font_load(&sdfont, filename, pixsize)) {
        sd_font_free(&sdfont);
        return 2;
    }

    // Load into textures
    sd_vga_image_create(&img, pixsize, pixsize);
    for(int i = 0; i < 224; i++) {
        sur = omf_calloc(1, sizeof(surface));
        sd_font_decode(&sdfont, &img, i, 0);
        surface_create_from_vga(sur, &img);
        vector_append(&font->surfaces, &sur);
    }

    // Set font info vars
    font->w = pixsize;
    font->h = pixsize;
    font->size = size;

    // Free resources
    sd_vga_image_free(&img);
    sd_font_free(&sdfont);
    return 0;
}

int pcx_font_load(font *font, const char *filename, int8_t palette_offset) {
    //sd_rgba_image img;
    pcx_font pcx_font;
    int pixsize;
    //surface *sur;

    if(pcx_load_font(&pcx_font, filename)) {
        pcx_font_free(&pcx_font);
        return 1;
    }

    pixsize = pcx_font.glyph_height;

    /* FIXME
    // Load into textures
    for(int i = 0; i < pcx_font.glyph_count; i++) {
        sd_rgba_image_create(&img, pcx_font.glyphs[i].width, pixsize);
        sur = omf_calloc(1, sizeof(surface));
        pcx_font_decode(&pcx_font, &img, i, palette_offset);
        surface_create_from_data(sur, SURFACE_TYPE_RGBA, img.w, img.h, img.data);
        vector_append(&font->surfaces, &sur);
        sd_rgba_image_free(&img);
    }
    */

    // Set font info vars
    font->w = 0;
    font->h = pixsize;
    font->size = pixsize;

    // Free resources
    pcx_font_free(&pcx_font);
    return 0;
}

int fonts_init(void) {
    font_create(&font_small);
    font_create(&font_large);
    font_create(&font_net1);
    font_create(&font_net2);
    const char *filename = NULL;

    // Load small font
    filename = pm_get_resource_path(DAT_CHARSMAL);
    if(font_load(&font_small, filename, FONT_SMALL)) {
        PERROR("Unable to load font file '%s'!", filename);
        goto error_4;
    }
    INFO("Loaded font file '%s'", filename);

    // Load big font
    filename = pm_get_resource_path(DAT_GRAPHCHR);
    if(font_load(&font_large, filename, FONT_BIG)) {
        PERROR("Unable to load font file '%s'!", filename);
        goto error_3;
    }
    INFO("Loaded font file '%s'", filename);

    // Load big net font
    filename = pm_get_resource_path(PCX_NETFONT1);
    if(pcx_font_load(&font_net1, filename, 3)) {
        PERROR("Unable to load font file '%s'!", filename);
        goto error_2;
    }
    INFO("Loaded font file '%s'", filename);

    // Load small net font
    filename = pm_get_resource_path(PCX_NETFONT2);
    if(pcx_font_load(&font_net2, filename, 16)) {
        PERROR("Unable to load font file '%s'!", filename);
        goto error_1;
    }
    INFO("Loaded font file '%s'", filename);

    // All done.
    fonts_loaded = 1;
    return 0;

error_1:
    font_free(&font_net2);
error_2:
    font_free(&font_net1);
error_3:
    font_free(&font_large);
error_4:
    font_free(&font_small);
    return 1;
}

void fonts_close(void) {
    if(fonts_loaded) {
        font_free(&font_small);
        font_free(&font_large);
        fonts_loaded = 0;
    }
}
