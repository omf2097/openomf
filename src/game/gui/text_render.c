#include <string.h>

#include "game/gui/text_render.h"
#include "video/video.h"
#include "utils/vector.h"
#include "utils/log.h"

void text_defaults(text_settings *settings) {
    memset(settings, 0, sizeof(text_settings));
    settings->cforeground = color_create(0xFF,0xFF,0xFF,0xFF);
}

static void text_render_char(text_settings *settings, int x, int y, char ch) {
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
    if(settings->shadow & TEXT_SHADOW_RIGHT)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x+1, y, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, 80, settings->cforeground
        );
    if(settings->shadow & TEXT_SHADOW_LEFT)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x-1, y, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, 80, settings->cforeground
        );
    if(settings->shadow & TEXT_SHADOW_BOTTOM)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x, y+1, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, 80, settings->cforeground
        );
    if(settings->shadow & TEXT_SHADOW_TOP)
        video_render_sprite_flip_scale_opacity_tint(
            *sur, x, y-1, BLEND_ALPHA, 0, FLIP_NONE, 1.0f, 80, settings->cforeground
        );

    // Handle the font face itself
    video_render_sprite_tint(*sur, x, y, settings->cforeground, 0);
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
                return last_space+1;
            }
        } else if(ptr[i] == ' ') {
            last_space = i;
        }
    }

    return i;
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

void text_render(text_settings *settings, int x, int y, int w, int h, const char *text) {
    int len = strlen(text);

    int size = (settings->font == FONT_BIG) ? 8 : 6;
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
                    start_x += (xspace - tmp_s) / 2;
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
                    start_y += (yspace - tmp_s) / 2;
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
                line_ph = charh;
                my += charh * line;

                // Horizontal alignment for this line
                switch(settings->halign) {
                    case TEXT_CENTER:
                        mx += (xspace - line_pw) / 2;
                        break;
                    case TEXT_RIGHT:
                        mx += (xspace - line_pw);
                        break;
                    default: break;
                }
                break;
            case TEXT_VERTICAL:
                line_pw = charw;
                line_ph = real_len * charh - settings->lspacing;
                mx += charw * line;

                // Vertical alignment for this line
                switch(settings->valign) {
                    case TEXT_MIDDLE:
                        my += (yspace - line_ph) / 2;
                        break;
                    case TEXT_BOTTOM:
                        my += (yspace - line_ph);
                        break;
                    default: break;
                }
                break;
        }

        // Render characters
        DEBUG("%d,%d = %s", mx, my, (char*)(text+ptr));
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
