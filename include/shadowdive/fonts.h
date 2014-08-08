#ifndef _SD_FONTS_H
#define _SD_FONTS_H

#include <stdint.h>
#include "shadowdive/rgba_image.h"

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    char data[8];
} sd_char;

typedef struct {
    unsigned int h;
    sd_char chars[224];
} sd_font;

int sd_font_create(sd_font *font);
void sd_font_free(sd_font *font);
int sd_font_load(sd_font *font, const char *file, unsigned int font_h);
int sd_font_save(sd_font *font, const char *file);
int sd_font_decode(
    sd_font *font,
    sd_rgba_image* surface,
    uint8_t ch,
    uint8_t r,
    uint8_t g,
    uint8_t b);

#ifdef __cplusplus
}
#endif

#endif // _SD_FONTS_H
