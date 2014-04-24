#include <shadowdive/shadowdive.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "game/text/text.h"
#include "video/video.h"
#include "utils/log.h"
#include "resources/ids.h"

font font_small;
font font_large;

void font_create(font *font) {
    font->size = FONT_UNDEFINED;
    vector_create(&font->surfaces, sizeof(surface*));
}

void font_free(font *font) {
    font->size = FONT_UNDEFINED;
    iterator it;
    vector_iter_begin(&font->surfaces, &it);
    surface **sur = NULL;
    while((sur = iter_next(&it)) != NULL) {
        surface_free(*sur);
        free(*sur);
    }
    vector_free(&font->surfaces);
}

int font_load(font *font, const char* filename, unsigned int size) {
    sd_rgba_image *img;
    sd_font *sdfont;
    int pixsize;
    surface *sur;

    // Find vertical size
    switch(size) {
        case FONT_BIG: pixsize = 8; break;
        case FONT_SMALL: pixsize = 6; break;
        default:
            return 1;
    }

    // Open font file
    sdfont = sd_font_create();
    if(sd_font_load(sdfont, filename, pixsize)) {
        sd_font_delete(sdfont);
        return 2;
    }

    // Load into textures
    img = sd_rgba_image_create(pixsize, pixsize);
    for(int i = 0; i < 224; i++) {
        sur = malloc(sizeof(surface));
        sd_font_decode(sdfont, img, i, 0xFF, 0xFF, 0xFF);
        surface_create_from_data(sur, SURFACE_TYPE_RGBA, img->w, img->h, img->data);
        vector_append(&font->surfaces, &sur);
    }

    // Set font info vars
    font->w = pixsize;
    font->h = pixsize;
    font->size = size;

    // Free resources
    sd_rgba_image_delete(img);
    sd_font_delete(sdfont);
    return 0;
}

int fonts_init() {
    font_create(&font_small);
    font_create(&font_large);
    char *filename = NULL;

    // Load small font
    filename = get_path_by_id(DAT_CHARSMAL);
    if(font_load(&font_small, filename, FONT_SMALL)) {
        PERROR("Unable to load font file '%s'!", filename);
        free(filename);
        return 1;
    }
    INFO("Loaded font file '%s'", filename);
    free(filename);

    // Load big font
    filename = get_path_by_id(DAT_GRAPHCHR);
    if(font_load(&font_large, filename, FONT_BIG)) {
        PERROR("Unable to load font file '%s'!", filename);
        free(filename);
        return 1;
    }
    INFO("Loaded font file '%s'", filename);
    free(filename);

    // All done.
    return 0;
}

void fonts_close() {
    font_free(&font_small);
    font_free(&font_large);
}

void font_get_wrapped_size(font *font, const char *text, int max_w, int *out_w, int *out_h) {
    int len = strlen(text);
    int has_newline = 0;
    for(int i = 0;i < len;i++) {
        if(text[i] == '\n' || text[i] == '\r') {
            has_newline = 1;
            break;
        }
    }
    if (!has_newline && font->w*len < max_w) {
        // short enough text that we don't need to wrap
        *out_w = font->w*len;
        *out_h = font->h;
    } else {
        // ok, we actually have to do some real work
        // look ma, no mallocs!
        const char *start = text;
        const char *stop = start;
        const char *end = &start[len];
        const char *tmpstop;
        int maxlen = max_w/font->w;
        int yoff = 0;
        int is_last_line = 0;

        *out_w = 0;
        *out_h = 0;
        while (start != end) {
            stop = tmpstop = start;
            while(1) {
                // rules:
                // 1. split lines by whitespaces
                // 2. pack as many words as possible into a line
                // 3. a line must be no more than maxlen long
                if(*stop == 0) {
                    // the end
                    is_last_line = 1;
                    stop--;
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
            int linelen = (stop - start) + (is_last_line?1:0);
            if(*out_w < linelen*font->w) {
                *out_w = linelen*font->w;
            }
            yoff += font->h;
            start = stop+1;
            stop = start;
        }
        *out_h = yoff;
    }
}

void font_render_char(font *font, char ch, int x, int y, color c) {
    font_render_char_shadowed(font, ch, x, y, c, 0);
}

void font_render_char_shadowed(font *font, char ch, int x, int y, color c, int shadow_flags) {
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

void font_render_len(font *font, const char *text, int len, int x, int y, color c) {
    font_render_len_shadowed(font, text, len, x, y, c, 0);
}

void font_render_len_shadowed(font *font, const char *text, int len, int x, int y, color c, int shadow_flags) {
    int pos_x = x;
    for(int i = 0; i < len; i++) {
        font_render_char_shadowed(font, text[i], pos_x, y, c, shadow_flags);
        pos_x += font->w;
    }
}

void font_render(font *font, const char *text, int x, int y, color c) {
    int len = strlen(text);
    font_render_len(font, text, len, x, y, c);
}

void font_render_shadowed(font *font, const char *text, int x, int y, color c, int shadow_flags) {
    int len = strlen(text);
    font_render_len_shadowed(font, text, len, x, y, c, shadow_flags);
}

void font_render_wrapped(font *font, const char *text, int x, int y, int w, color c) {
    font_render_wrapped_shadowed(font, text, x, y, w, c, 0);
}

// XXX If you modify this function please also reflect the changes onto font_get_wrapped_size().
void font_render_wrapped_shadowed(font *font, const char *text, int x, int y, int w, color c, int shadow_flags) {
    int len = strlen(text);
    int has_newline = 0;
    for(int i = 0;i < len;i++) {
        if(text[i] == '\n' || text[i] == '\r') {
            has_newline = 1;
            break;
        }
    }
    if(!has_newline && font->w*len < w) {
        // short enough text that we don't need to wrap

        // render it centered, at least for now
        int xoff = (w - font->w*len)/2;
        font_render_len_shadowed(font, text, len, x + xoff, y, c, shadow_flags);
    } else {
        // ok, we actually have to do some real work
        // look ma, no mallocs!
        const char *start = text;
        const char *stop = start;
        const char *end = &start[len];
        const char *tmpstop;
        int maxlen = w/font->w;
        int yoff = 0;
        int is_last_line = 0;

        while(start != end) {
            stop = tmpstop = start;
            while(1) {
                // rules:
                // 1. split lines by whitespaces
                // 2. pack as many words as possible into a line
                // 3. a line must be no more than maxlen long
                if(*stop == 0) {
                    // the end
                    is_last_line = 1;
                    stop--;
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
            int xoff = (w - font->w*linelen)/2;
            if(shadow_flags & TEXT_SHADOW_TOP) {
                yoff++;
            }
            font_render_len_shadowed(font, start, linelen + (is_last_line?1:0), x + xoff, y + yoff, c, shadow_flags);
            yoff += font->h;
            if(shadow_flags & TEXT_SHADOW_BOTTOM) {
                yoff++;
            }
            start = stop+1;
            stop = start;
        }
    }
}
