#ifndef _FONTS_H
#define _FONTS_H

typedef struct sd_rgba_image_t sd_rgba_image;

#include <stdint.h>

typedef struct sd_char_big_t {
    char data[8];
} sd_char_big;

typedef struct sd_char_small_t {
    char data[6];
} sd_char_small;

typedef struct sd_font_big_t {
    sd_char_big chars[224];
} sd_font_big;

typedef struct sd_font_small_t {
    sd_char_small chars[224];
} sd_font_small;

sd_font_small* sd_font_small_create();
sd_font_big* sd_font_big_create();
void sd_font_small_delete(sd_font_small *font);
void sd_font_big_delete(sd_font_big *font);

int sd_font_small_load(sd_font_small *font, const char *file);
int sd_font_big_load(sd_font_big *font, const char *file);

sd_rgba_image* sd_font_big_decode(sd_char_big *ch, uint32_t color);
sd_rgba_image* sd_font_small_decode(sd_char_small *ch, uint32_t color);

#endif // _FONTS_H