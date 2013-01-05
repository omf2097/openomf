#include "shadowdive/fonts.h"
#include "shadowdive/rgba_image.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/error.h"
#include <stdlib.h>

#include <stdio.h>

sd_font_small* sd_font_small_create() {
    return (sd_font_small*)malloc(sizeof(sd_font_small));
}

sd_font_big* sd_font_big_create() {
    return (sd_font_big*)malloc(sizeof(sd_font_big));
}

void sd_font_small_delete(sd_font_small *font) {
    if(font) { free(font); }
}

void sd_font_big_delete(sd_font_big *font) {
    if(font) { free(font); }
}

int sd_font_big_load(sd_font_big *font, const char *file) {
    sd_reader *r = sd_reader_open(file);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }
    
    for(int i = 0; i < 224; i++) {
        sd_read_buf(r, font->chars[i].data, 8);
        if(!sd_reader_ok(r)) {
            sd_reader_close(r);
            return SD_FILE_PARSE_ERROR;
        }
    }
    
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_font_small_load(sd_font_small *font, const char *file) {
    sd_reader *r = sd_reader_open(file);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }
    
    for(int i = 0; i < 224; i++) {
        sd_read_buf(r, font->chars[i].data, 8);
        if(!sd_reader_ok(r)) {
            sd_reader_close(r);
            return SD_FILE_PARSE_ERROR;
        }
    }
    
    sd_reader_close(r);
    return SD_SUCCESS;
}

sd_rgba_image* sd_font_big_decode(sd_char_big *ch, uint32_t color) {
    sd_rgba_image *o = sd_rgba_image_create(8, 6);

    return o;
}

sd_rgba_image* sd_font_small_decode(sd_char_small *ch, uint32_t color) {
    sd_rgba_image *o = sd_rgba_image_create(8, 8);
    
    return o;
}