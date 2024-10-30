#include <string.h>

#include "formats/error.h"
#include "formats/fonts.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"

int sd_font_create(sd_font *font) {
    if(font == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(font, 0, sizeof(sd_font));
    return SD_SUCCESS;
}

void sd_font_free(sd_font *font) {
}

int sd_font_load(sd_font *font, const char *file, unsigned int font_h) {
    if(font == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(file);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    font->h = font_h;
    for(int i = 0; i < 224; i++) {
        sd_read_buf(r, font->chars[i].data, font_h);
        if(!sd_reader_ok(r)) {
            sd_reader_close(r);
            return SD_FILE_PARSE_ERROR;
        }
    }

    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_font_save(const sd_font *font, const char *file) {
    if(font == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(file);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    for(int i = 0; i < 224; i++) {
        sd_write_buf(w, font->chars[i].data, font->h);
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

int sd_font_decode(const sd_font *font, sd_vga_image *o, uint8_t ch, uint8_t color) {
    if(font == NULL || o == NULL || ch >= 224) {
        return SD_INVALID_INPUT;
    }

    int t = 0;
    for(unsigned i = 0; i < font->h; i++) {
        for(int k = font->h - 1; k >= 0; k--) {
            if(font->chars[ch].data[i] & (1 << k)) {
                o->data[t] = color;
            } else {
                o->data[t] = 0;
            }
            t++;
        }
    }
    return SD_SUCCESS;
}

int sd_font_decode_rgb(const sd_font *font, sd_rgba_image *o, uint8_t ch, uint8_t r, uint8_t g, uint8_t b) {
    if(font == NULL || o == NULL || ch >= 224) {
        return SD_INVALID_INPUT;
    }

    int t = 0;
    for(unsigned int i = 0; i < font->h; i++) {
        for(int k = font->h - 1; k >= 0; k--) {
            if(font->chars[ch].data[i] & (1 << k)) {
                o->data[t++] = r;
                o->data[t++] = g;
                o->data[t++] = b;
                o->data[t++] = (uint8_t)255;
            } else {
                o->data[t++] = 0;
                o->data[t++] = 0;
                o->data[t++] = 0;
                o->data[t++] = 0;
            }
        }
    }
    return SD_SUCCESS;
}
