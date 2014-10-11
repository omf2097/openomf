#ifndef _TEXT_RENDER_H
#define _TEXT_RENDER_H

#include <stdint.h>
#include <stdbool.h>
#include "video/color.h"
#include "resources/fonts.h"

typedef enum {
    TEXT_TOP = 0,
    TEXT_MIDDLE,
    TEXT_BOTTOM
} text_valign;

typedef enum {
    TEXT_LEFT = 0,
    TEXT_CENTER,
    TEXT_RIGHT
} text_halign;

typedef struct {
    uint8_t left;
    uint8_t right;
    uint8_t top;
    uint8_t bottom;
} text_padding;

typedef enum {
    TEXT_HORIZONTAL = 0,
    TEXT_VERTICAL,
} text_direction;

#ifndef TEXT_SHADOW_DEF
#define TEXT_SHADOW_DEF
enum {
    TEXT_SHADOW_NONE = 0,
    TEXT_SHADOW_TOP = 0x1,
    TEXT_SHADOW_BOTTOM = 0x2,
    TEXT_SHADOW_LEFT = 0x4,
    TEXT_SHADOW_RIGHT = 0x8,
    TEXT_SHADOW_HORIZONTAL = 0xC,
    TEXT_SHADOW_VERTICAL = 0x3,
    TEXT_SHADOW_ALL = 0xF
};
#endif // TEXT_SHADOW_DEF

typedef struct {
    color cforeground;
    text_valign valign;
    text_halign halign;
    text_padding padding;
    text_direction direction;
    font_size font;
    uint8_t shadow;
    uint8_t cspacing;
    uint8_t lspacing;
    bool wrap;
} text_settings;

void text_defaults(text_settings *settings);
int text_find_max_strlen(int maxchars, const char *ptr);
int text_find_line_count(text_direction dir, int cols, int rows, int len, const char *text);
void text_render(text_settings *settings, int x, int y, int w, int h, const char *text);

#endif // _TEXT_RENDER_H
