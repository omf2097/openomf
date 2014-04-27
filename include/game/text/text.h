#ifndef _TEXT_H
#define _TEXT_H

#include "utils/vector.h"
#include "video/color.h"
#include "resources/fonts.h"

enum {
    TEXT_SHADOW_TOP = 0x1,
    TEXT_SHADOW_BOTTOM = 0x2,
    TEXT_SHADOW_LEFT = 0x4,
    TEXT_SHADOW_RIGHT = 0x8,
    TEXT_SHADOW_HORIZONTAL = 0xC,
    TEXT_SHADOW_VERTICAL = 0x3,
    TEXT_SHADOW_ALL = 0xF
};

void font_get_wrapped_size(font *font, const char *text, int max_w, int *out_w, int *out_h);
void font_render_char(font *font, char ch, int x, int y, color c);
void font_render_char_shadowed(font *font, char ch, int x, int y, color c, int shadow_flags);
void font_render_len(font *font, const char *text, int len, int x, int y, color c);
void font_render_len_shadowed(font *font, const char *text, int len, int x, int y, color c, int shadow_flags);
void font_render(font *font, const char *text, int x, int y, color c);
void font_render_shadowed(font *font, const char *text, int x, int y, color c, int shadow_flags);
void font_render_wrapped(font *font, const char *text, int x, int y, int w, color c);
void font_render_wrapped_shadowed(font *font, const char *text, int x, int y, int w, color c, int shadow_flags);

#endif // _TEXT_H
