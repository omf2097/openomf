#include <string.h>
#include <ctype.h>
#include <math.h>

#include "game/gui/text_render.h"
#include "video/video.h"
#include "utils/vector.h"
#include "utils/log.h"

void text_defaults(text_settings *settings) {
    memset(settings, 0, sizeof(text_settings));
    settings->cforeground = color_create(0xFF,0xFF,0xFF,0xFF);
    settings->opacity = 0xFF;
}

void text_render_char(const text_settings *settings, int x, int y, char ch) {
    // Make sure code is valid
    int code = ch - 32;
    surface **sur = NULL;
    if(code < 0) {
        return;
    }

    // Select font face surface
    if(settings->font == FONT_BIG) {
        sur = vector_get(&font_large.surfaces, code);
    }
    if(settings->font == FONT_SMALL) {
        sur = vector_get(&font_small.surfaces, code);
    }
    if(sur == NULL || *sur == NULL) {
        return;
    }

    // Handle shadows if necessary
    float of = settings->opacity / 255.0f;
    if(settings->shadow & TEXT_SHADOW_RIGHT)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x+1, y, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, of * 80, settings->cforeground
        );
    if(settings->shadow & TEXT_SHADOW_LEFT)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x-1, y, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, of * 80, settings->cforeground
        );
    if(settings->shadow & TEXT_SHADOW_BOTTOM)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x, y+1, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, of * 80, settings->cforeground
        );
    if(settings->shadow & TEXT_SHADOW_TOP)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x, y-1, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, of * 80, settings->cforeground
        );

    // Handle the font face itself
    video_render_sprite_flip_scale_opacity_tint(
        *sur, x, y, BLEND_ALPHA, 0, FLIP_NONE, 1, settings->opacity, settings->cforeground);
}

int text_find_max_strlen(int maxchars, const char *ptr) {
    int i;
    int len = strlen(ptr);

    // Skip whitespace at the start of the string
    for(i = 0; i < len; i++) {
        if(ptr[i] != ' ')
            break;
    }
    if(i == len-1) {
        return i;
    }

    // Walk through the rest of the string
    int last_space = i;
    int max = maxchars + i;
    int lstart = i;
    for(; i < len; i++) {
        // If we detect newline, this line ends here
        if(ptr[i] == '\n')
            return i+1;

        // If we are reading over our text limit ...
        if(i >= max) {
            if(ptr[i] == ' ') { // If we are at valid end character (space), end here
                return i;
            } else if(last_space != lstart) {
                return last_space;
            }
        } else if(ptr[i] == ' ') {
            last_space = i;
        }
    }

    return i;
}

int text_char_width(const text_settings *settings) {
    return (settings->font == FONT_BIG) ? 8 : 6;
}

int text_find_line_count(text_direction dir, int cols, int rows, int len, const char *text) {
    int ptr = 0;
    int lines = 0;
    while(ptr < len-1) {
        // Find out how many characters for this row/col
        int line_len;
        if(dir == TEXT_HORIZONTAL)
            line_len = text_find_max_strlen(cols, text + ptr);
        else
            line_len = text_find_max_strlen(rows, text + ptr);

        ptr += line_len;
        lines++;
    }
    return lines;
}

void text_render(const text_settings *settings, int x, int y, int w, int h, const char *text) {
    int len = strlen(text);

    int size = text_char_width(settings);
    int xspace = w - settings->padding.left - settings->padding.right;
    int yspace = h - settings->padding.top - settings->padding.bottom;
    int charw = size + settings->cspacing;
    int charh = size + settings->lspacing;
    int rows = (yspace + settings->lspacing) / charh;
    int cols = (xspace + settings->cspacing) / charw;
    int fit_lines = text_find_line_count(settings->direction, cols, rows, len, text);

    int start_x = x + settings->padding.left;
    int start_y = y + settings->padding.top;
    int tmp_s = 0;

    // Initial alignment for whole text block
    switch(settings->direction) {
        case TEXT_VERTICAL:
            tmp_s = fit_lines * charw - settings->cspacing; // Total W minus last spacing
            switch(settings->halign) {
                case TEXT_CENTER:
                    start_x += ceil((xspace - tmp_s) / 2.0f);
                    break;
                case TEXT_RIGHT:
                    start_x += (xspace - tmp_s);
                    break;
                default: break;
            }
            break;
        case TEXT_HORIZONTAL:
            tmp_s = fit_lines * charh - settings->lspacing; // Total H minus last spacing
            switch(settings->valign) {
                case TEXT_MIDDLE:
                    start_y += floor((yspace - tmp_s) / 2.0f);
                    break;
                case TEXT_BOTTOM:
                    start_y += (yspace - tmp_s);
                    break;
                default: break;
            }
            break;
    }

    int ptr = 0;
    int line = 0;
    while(ptr < len-1 && line < fit_lines) {
        int line_len;
        int real_len;
        int mx = 0;
        int my = 0;
        int line_pw;
        int line_ph;

        // Find out how many characters for this row/col
        if(settings->direction == TEXT_HORIZONTAL)
            line_len = text_find_max_strlen(cols, text + ptr);
        else
            line_len = text_find_max_strlen(rows, text + ptr);
        real_len = line_len;

        // Skip spaces
        int k = 0;
        for(; k < line_len; k++) {
            if(text[ptr+k] != ' ')
                break;
            real_len--;
        }

        // Find total size of this line and set newline start coords
        switch(settings->direction) {
            case TEXT_HORIZONTAL:
                line_pw = real_len * charw - settings->cspacing;
                my += charh * line;

                // Horizontal alignment for this line
                switch(settings->halign) {
                    case TEXT_CENTER:
                        mx += floor((xspace - line_pw) / 2.0f);
                        break;
                    case TEXT_RIGHT:
                        mx += (xspace - line_pw);
                        break;
                    default: break;
                }
                break;
            case TEXT_VERTICAL:
                line_ph = real_len * charh - settings->lspacing;
                mx += charw * line;

                // Vertical alignment for this line
                switch(settings->valign) {
                    case TEXT_MIDDLE:
                        my += ceil((yspace - line_ph) / 2.0f);
                        break;
                    case TEXT_BOTTOM:
                        my += (yspace - line_ph);
                        break;
                    default: break;
                }
                break;
        }

        // Render characters
        for(; k < line_len; k++) {
            // Skip line endings.
            if(text[ptr+k] == '\n')
                continue;

            // Render character
            text_render_char(settings, mx + start_x, my + start_y, text[ptr+k]);

            // Render to the right direction
            if(settings->direction == TEXT_HORIZONTAL) {
                mx += charw;
            } else {
                my += charh;
            }
        }

        ptr += line_len;
        line++;
    }
}

/// ---------------- OLD RENDERER FUNCTIONS ---------------------

void font_render_char(const font *font, char ch, int x, int y, color c) {
    font_render_char_shadowed(font, ch, x, y, c, 0);
}

void font_render_char_shadowed(const font *font, char ch, int x, int y, color c, int shadow_flags) {
    // Make sure code is valid
    int code = ch - 32;
    surface **sur = NULL;
    if (code < 0) {
        return;
    }

    // Get font face
    sur = vector_get(&font->surfaces, code);

    // Handle shadows if necessary
    if(shadow_flags & TEXT_SHADOW_RIGHT)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x+1, y, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, 80, c
        );
    if(shadow_flags & TEXT_SHADOW_LEFT)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x-1, y, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, 80, c
        );
    if(shadow_flags & TEXT_SHADOW_BOTTOM)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x, y+1, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, 80, c
        );
    if(shadow_flags & TEXT_SHADOW_TOP)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x, y-1, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, 80, c
        );

    // Handle the font face itself
    video_render_sprite_tint(*sur, x, y, c, 0);
}

void font_render_len(const font *font, const char *text, int len, int x, int y, color c) {
    font_render_len_shadowed(font, text, len, x, y, c, 0);
}

void font_render_len_shadowed(const font *font, const char *text, int len, int x, int y, color c, int shadow_flags) {
    int pos_x = x;
    for(int i = 0; i < len; i++) {
        font_render_char_shadowed(font, text[i], pos_x, y, c, shadow_flags);
        pos_x += font->w;
    }
}

void font_render(const font *font, const char *text, int x, int y, color c) {
    int len = strlen(text);
    font_render_len(font, text, len, x, y, c);
}

void font_render_shadowed(const font *font, const char *text, int x, int y, color c, int shadow_flags) {
    int len = strlen(text);
    font_render_len_shadowed(font, text, len, x, y, c, shadow_flags);
}

void font_render_wrapped(const font *font, const char *text, int x, int y, int w, color c) {
    font_render_wrapped_shadowed(font, text, x, y, w, c, 0);
}

void font_render_wrapped_internal(const font *font, const char *text, int x, int y, int max_w, color c, int shadow_flags, int only_size, int *out_w, int *out_h) {
    int len = strlen(text);
    int has_newline = 0;
    for(int i = 0;i < len;i++) {
        if(text[i] == '\n' || text[i] == '\r') {
            has_newline = 1;
            break;
        }
    }
    if(!has_newline && font->w*len < max_w) {
        // short enough text that we don't need to wrap
        // render it centered, at least for now
        if(!only_size) {
            int xoff = (max_w - font->w*len)/2;
            font_render_len_shadowed(font, text, len, x + xoff, y, c, shadow_flags);
        }
        *out_w = font->w*len;
        *out_h = font->h;
    } else {
        // ok, we actually have to do some real work
        // look ma, no mallocs!
        const char *start = text;
        const char *stop;
        const char *end = &start[len];
        const char *tmpstop;
        int maxlen = max_w/font->w;
        int yoff = 0;
        int is_last_line = 0;

        *out_w = 0;
        *out_h = 0;
        while(start != end) {
            stop = tmpstop = start;
            while(1) {
                // rules:
                // 1. split lines by whitespaces
                // 2. pack as many words as possible into a line
                // 3. a line must be no more than maxlen long
                if(*stop == 0) {
                    // hit the end
                    if(stop - start > maxlen) {
                        // the current line exceeds max len
                        if(tmpstop - start > maxlen) {
                            // this line cannot not be word-wrapped because it contains a word that exceeds maxlen, we'll let it pass
                            stop--;
                            is_last_line = 1;
                        } else {
                            // this line can be word-wrapped, go back to previous saved location
                            stop = tmpstop;
                        }
                    } else {
                        stop--;
                        is_last_line = 1;
                    }
                    break;
                }
                if(*stop == '\n' || *stop == '\r') {
                    if(stop - start > maxlen) {
                        stop = tmpstop;
                    }
                    break;
                }
                if(isspace(*stop)) {
                    if(stop - start > maxlen) {
                        stop = tmpstop;
                        break;
                    } else {
                        tmpstop = stop;
                    }
                }
                stop++;
            }
            int linelen = stop - start;
            if(shadow_flags & TEXT_SHADOW_TOP) {
                yoff++;
            }
            if(!only_size) {
                int xoff = (max_w - font->w*linelen)/2;
                font_render_len_shadowed(font, start, linelen + (is_last_line?1:0), x + xoff, y + yoff, c, shadow_flags);
            }
            if(*out_w < linelen*font->w) {
                *out_w = linelen*font->w;
            }
            yoff += font->h;
            if(shadow_flags & TEXT_SHADOW_BOTTOM) {
                yoff++;
            }
            *out_h = yoff;
            start = stop+1;
        }
    }
}

void font_render_wrapped_shadowed(const font *font, const char *text, int x, int y, int w, color c, int shadow_flags) {
    int tmp;
    font_render_wrapped_internal(font, text, x, y, w, c, shadow_flags, 0, &tmp, &tmp);
}

void font_get_wrapped_size(const font *font, const char *text, int max_w, int *out_w, int *out_h) {
    static color c = {0};
    font_render_wrapped_internal(font, text, 0, 0, max_w, c, 0, 1, out_w, out_h);
}

void font_get_wrapped_size_shadowed(const font *font, const char *text, int max_w, int shadow_flag, int *out_w, int *out_h) {
    static color c = {0};
    font_render_wrapped_internal(font, text, 0, 0, max_w, c, shadow_flag, 1, out_w, out_h);
}
