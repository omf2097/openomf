#ifndef _SD_FONTS_H
#define _SD_FONTS_H

#ifndef _SD_RGBA_IMAGE_H
typedef struct sd_rgba_image_t sd_rgba_image;
#endif

#include <stdint.h>

typedef struct sd_char_t {
    char data[8];
} sd_char;

typedef struct sd_font_t {
    unsigned int h;
    sd_char chars[224];
} sd_font;

sd_font* sd_font_create();
void sd_font_delete(sd_font *font);
int sd_font_load(sd_font *font, const char *file, unsigned int font_h);
int sd_font_save(sd_font *font, const char *file);
int sd_font_decode(sd_font *font, sd_rgba_image* surface, uint8_t ch, uint8_t r, uint8_t g, uint8_t b);

#endif // _SD_FONTS_H
