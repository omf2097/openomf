#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "game/text/text.h"
#include "video/video.h"
#include "utils/log.h"

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
