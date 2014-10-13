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
    uint8_t opacity;
    bool wrap;
} text_settings;

// New text rendering functions
void text_defaults(text_settings *settings);
int text_find_max_strlen(int maxchars, const char *ptr);
int text_find_line_count(text_direction dir, int cols, int rows, int len, const char *text);
void text_render_char(const text_settings *settings, int x, int y, char ch);
void text_render(const text_settings *settings, int x, int y, int w, int h, const char *text);

// Old functions
void font_get_wrapped_size(const font *font, const char *text, int max_w, int *out_w, int *out_h);
void font_get_wrapped_size_shadowed(const font *font, const char *text, int max_w, int shadow_flag, int *out_w, int *out_h);
void font_render_char(const font *font, char ch, int x, int y, color c);
void font_render_char_shadowed(const font *font, char ch, int x, int y, color c, int shadow_flags);
void font_render_len(const font *font, const char *text, int len, int x, int y, color c);
void font_render_len_shadowed(const font *font, const char *text, int len, int x, int y, color c, int shadow_flags);
void font_render(const font *font, const char *text, int x, int y, color c);
void font_render_shadowed(const font *font, const char *text, int x, int y, color c, int shadow_flags);
void font_render_wrapped(const font *font, const char *text, int x, int y, int w, color c);
void font_render_wrapped_shadowed(const font *font, const char *text, int x, int y, int w, color c, int shadow_flags);

#endif // _TEXT_RENDER_H
