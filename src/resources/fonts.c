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

static font font_small;
static font font_large;
static font font_net1;
static font font_net2;
static int fonts_loaded = 0;
static unsigned char FIRST_PRINTABLE_CHAR = (unsigned char)' ';

static void free_glyph(void *d) {
    surface *s = (surface *)d;
    surface_free(s);
}

void font_create(font *f) {
    memset(f, 0, sizeof(font));
    vector_create_with_size_cb(&f->surfaces, sizeof(surface), 233, free_glyph);
}

void font_free(font *font) {
    vector_free(&font->surfaces);
}

static int text_char_to_glyph_index(char c) {
    int ic = (int)(unsigned char)c;
    return ic - FIRST_PRINTABLE_CHAR;
}

const surface *font_get_surface(const font *font, char ch) {
    int code = text_char_to_glyph_index(ch);
    if(code < 0) {
        return NULL;
    }
    return vector_get(&font->surfaces, code);
}

static int font_load(font *font, const char *filename, unsigned int size) {
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
        sd_font_decode(&sdfont, &img, i, 1);
        sur = vector_append_ptr(&font->surfaces);
        surface_create_from_vga(sur, &img);
        surface_set_transparency(sur, 0);
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

static int pcx_font_load(font *font, const char *filename, int8_t palette_offset) {
    sd_vga_image img;
    pcx_font pcx_font;
    int pixsize;
    surface *sur;

    if(pcx_load_font(&pcx_font, filename)) {
        pcx_font_free(&pcx_font);
        return 1;
    }

    pixsize = pcx_font.glyph_height;

    // Load into textures
    for(int i = 0; i < pcx_font.glyph_count; i++) {
        sd_vga_image_create(&img, pcx_font.glyphs[i].width, pixsize);
        pcx_font_decode(&pcx_font, &img, i, 1);
        sur = vector_append_ptr(&font->surfaces);
        surface_create_from_vga(sur, &img);
        surface_set_transparency(sur, 0);
        sd_vga_image_free(&img);
    }

    // Set font info vars
    font->w = 0;
    font->h = pixsize;
    font->size = pixsize;

    // Free resources
    pcx_font_free(&pcx_font);
    return 0;
}

bool fonts_init(void) {
    font_create(&font_small);
    font_create(&font_large);
    font_create(&font_net1);
    font_create(&font_net2);
    const char *filename = NULL;

    // Load small font
    filename = pm_get_resource_path(DAT_CHARSMAL);
    if(font_load(&font_small, filename, FONT_SMALL)) {
        log_error("Unable to load font file '%s'!", filename);
        goto error_4;
    }
    log_info("Loaded font file '%s'", filename);

    // Load big font
    filename = pm_get_resource_path(DAT_GRAPHCHR);
    if(font_load(&font_large, filename, FONT_BIG)) {
        log_error("Unable to load font file '%s'!", filename);
        goto error_3;
    }
    log_info("Loaded font file '%s'", filename);

    // Load big net font
    filename = pm_get_resource_path(PCX_NETFONT1);
    if(pcx_font_load(&font_net1, filename, 3)) {
        log_error("Unable to load font file '%s'!", filename);
        goto error_2;
    }
    log_info("Loaded font file '%s'", filename);

    // Load small net font
    filename = pm_get_resource_path(PCX_NETFONT2);
    if(pcx_font_load(&font_net2, filename, 16)) {
        log_error("Unable to load font file '%s'!", filename);
        goto error_1;
    }
    log_info("Loaded font file '%s'", filename);

    // All done.
    fonts_loaded = 1;
    return true;

error_1:
    font_free(&font_net2);
error_2:
    font_free(&font_net1);
error_3:
    font_free(&font_large);
error_4:
    font_free(&font_small);
    return false;
}

const font *fonts_get_font(font_size font) {
    switch(font) {
        case FONT_BIG:
            return &font_large;
        case FONT_SMALL:
            return &font_small;
        case FONT_NET1:
            return &font_net1;
        case FONT_NET2:
            return &font_net2;
        default:
            return NULL;
    }
}

void fonts_close(void) {
    if(fonts_loaded) {
        font_free(&font_small);
        font_free(&font_large);
        font_free(&font_net1);
        font_free(&font_net2);
        fonts_loaded = 0;
    }
}
