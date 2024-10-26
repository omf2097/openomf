#include <ctype.h>
#include <math.h>

#include "game/gui/text_render.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "video/video.h"

void text_defaults(text_settings *settings) {
    memset(settings, 0, sizeof(text_settings));
    settings->cforeground = 0xFD;
    settings->cselected = 0xFF;
    settings->cinactive = 0xFE;
    settings->cdisabled = 0xC0;
    settings->cshadow = 0xC0;
    settings->cspacing = 0;
    settings->max_lines = UINT8_MAX;
    settings->strip_leading_whitespace = false;
    settings->strip_trailing_whitespace = false;
}

int text_render_char(const text_settings *settings, text_mode state, int x, int y, char ch) {
    // Make sure code is valid
    int code = (unsigned char)ch - 32;
    surface **sur = NULL;
    if(code < 0) {
        return 0;
    }

    // Select font face surface
    if(settings->font == FONT_BIG) {
        sur = vector_get(&font_large.surfaces, code);
    }
    if(settings->font == FONT_SMALL) {
        sur = vector_get(&font_small.surfaces, code);
    }
    if(settings->font == FONT_NET1) {
        sur = vector_get(&font_net1.surfaces, code);
    }
    if(settings->font == FONT_NET2) {
        sur = vector_get(&font_net2.surfaces, code);
    }

    if(sur == NULL || *sur == NULL) {
        return 0;
    }

    // Handle shadows if necessary
    if(settings->shadow & TEXT_SHADOW_RIGHT)
        video_draw_offset(*sur, x + 1, y, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_LEFT)
        video_draw_offset(*sur, x - 1, y, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_BOTTOM)
        video_draw_offset(*sur, x, y + 1, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_TOP)
        video_draw_offset(*sur, x, y - 1, settings->cshadow, 255);

    int color;
    switch(state) {
        case TEXT_SELECTED:
            color = settings->cselected;
            break;
        case TEXT_UNSELECTED:
            color = settings->cinactive;
            break;
        case TEXT_DISABLED:
            color = settings->cdisabled;
            break;
        default:
            color = settings->cforeground;
    }

    // Draw the actual font
    video_draw_offset(*sur, x, y, color - 1, 255);
    return (*sur)->w;
}

int text_find_max_strlen(const text_settings *settings, int max_chars, const char *ptr) {
    int i = 0;
    int len = strlen(ptr);

    // Skip whitespace at the start of the string
    if(settings->strip_leading_whitespace) {
        for(i = 0; i < len; i++) {
            if(ptr[i] != ' ')
                break;
        }
        if(i == len) {
            return i;
        }
    }

    // Walk through the rest of the string
    int max = max_chars + i;
    int breakpoint = max;
    int next_breakpoint = breakpoint;
    for(; i < len; i++) {
        // If we detect newline, this line ends here
        if(ptr[i] == '\n') {
            return i + 1;
        }

        if(ptr[i] == ' ') {
            next_breakpoint = i + 1;
        } else {
            breakpoint = next_breakpoint;
        }

        if(i >= max) {
            return breakpoint;
        }
    }

    return i;
}

int text_width(const text_settings *settings, const char *text) {

    int len = strlen(text);
    int width = 0;
    int code = 0;
    surface **sur = NULL;
    for(int i = 0; i < len; i++) {
        code = text[i] - 32;
        if(code < 0) {
            continue;
        }
        // Select font face surface
        if(settings->font == FONT_BIG) {
            sur = vector_get(&font_large.surfaces, code);
        }
        if(settings->font == FONT_SMALL) {
            sur = vector_get(&font_small.surfaces, code);
        }
        if(settings->font == FONT_NET1) {
            sur = vector_get(&font_net1.surfaces, code);
        }
        if(settings->font == FONT_NET2) {
            sur = vector_get(&font_net2.surfaces, code);
        }
        if(sur == NULL || *sur == NULL) {
            continue;
        }
        width += (*sur)->w;
    }
    return width;
}

int text_char_width(const text_settings *settings) {
    // TODO this data is in the font itself, and we should handle variable width NET fonts, too
    switch(settings->font) {
        case FONT_BIG:
            return 8;
        case FONT_SMALL:
            return 6;
        case FONT_NET1:
            return 8;
        case FONT_NET2:
            return 6;
    }
    return 6;
}

int text_find_line_count(const text_settings *settings, int cols, int rows, int len, const char *text) {
    int ptr = 0;
    int lines = 0;
    while(lines < settings->max_lines && ptr < len) {
        // Find out how many characters for this row/col
        int line_len;
        if(settings->direction == TEXT_HORIZONTAL)
            line_len = text_find_max_strlen(settings, cols, text + ptr);
        else
            line_len = text_find_max_strlen(settings, rows, text + ptr);

        ptr += line_len;
        lines++;
    }
    return lines;
}

void text_render(const text_settings *settings, text_mode mode, int x, int y, int w, int h, const char *text) {
    int len = strlen(text);

    int size = text_char_width(settings);
    int xspace = w - settings->padding.left - settings->padding.right;
    int yspace = h - settings->padding.top - settings->padding.bottom;
    int charw = size + settings->cspacing;
    int charh = size + settings->lspacing;
    int rows = (yspace + settings->lspacing) / charh;
    int cols = (xspace + settings->cspacing) / charw;
    int fit_lines = text_find_line_count(settings, cols, rows, len, text);
    int max_chars = settings->direction == TEXT_HORIZONTAL ? cols : rows;
    if(max_chars == 0) {
        DEBUG("Warning: Text has zero size! text: '%s'");
        max_chars = 1;
    }

    int start_x = x + settings->padding.left;
    int start_y = y + settings->padding.top;
    int tmp_s = 0;

    // Initial alignment for whole text block
    switch(settings->direction) {
        case TEXT_VERTICAL:
            tmp_s = fit_lines * charw - settings->cspacing; // Total W minus last spacing
            switch(settings->halign) {
                case TEXT_CENTER:
                    start_x += ceilf((xspace - tmp_s) / 2.0f);
                    break;
                case TEXT_RIGHT:
                    start_x += (xspace - tmp_s);
                    break;
                default:
                    break;
            }
            break;
        case TEXT_HORIZONTAL:
            tmp_s = fit_lines * charh - settings->lspacing; // Total H minus last spacing
            switch(settings->valign) {
                case TEXT_MIDDLE:
                    start_y += floorf((yspace - tmp_s) / 2.0f);
                    break;
                case TEXT_BOTTOM:
                    start_y += (yspace - tmp_s);
                    break;
                default:
                    break;
            }
            break;
    }

    int ptr = 0;
    int line = 0;
    while(ptr < len && line < fit_lines) {
        int advance;
        int line_len;
        int mx = 0;
        int my = 0;
        int line_pw;
        int line_ph;

        // Find out how many characters for this row/col
        if(line + 1 == fit_lines)
            line_len = strlen(text + ptr);
        else
            line_len = text_find_max_strlen(settings, max_chars, text + ptr);
        advance = line_len;

        // If line ends in linebreak, skip it from calculation.
        if(text[ptr + line_len - 1] == '\n') {
            line_len--;
        }

        // Skip trailing spaces
        if(settings->strip_trailing_whitespace) {
            while(line_len > 0 && text[ptr + line_len - 1] == ' ') {
                line_len--;
            }
        }

        // Skip leading spaces
        int k = 0;
        if(settings->strip_leading_whitespace) {
            for(; k < line_len; k++) {
                if(text[ptr + k] != ' ')
                    break;
                line_len--;
            }
        }

        // Find total size of this line and set newline start coords
        switch(settings->direction) {
            case TEXT_HORIZONTAL:
                line_pw = line_len * charw - settings->cspacing;
                my += charh * line;

                // Horizontal alignment for this line
                switch(settings->halign) {
                    case TEXT_CENTER:
                        mx += floorf((xspace - line_pw) / 2.0f);
                        break;
                    case TEXT_RIGHT:
                        mx += (xspace - line_pw);
                        break;
                    default:
                        break;
                }
                break;
            case TEXT_VERTICAL:
                line_ph = line_len * charh - settings->lspacing;
                mx += charw * line;

                // Vertical alignment for this line
                switch(settings->valign) {
                    case TEXT_MIDDLE:
                        my += ceilf((yspace - line_ph) / 2.0f);
                        break;
                    case TEXT_BOTTOM:
                        my += (yspace - line_ph);
                        break;
                    default:
                        break;
                }
                break;
        }

        int w = 0;
        // Render characters
        for(; k < line_len; k++) {
            // Skip line endings.
            if(text[ptr + k] == '\n')
                continue;

            // Render character
            w = text_render_char(settings, mode, mx + start_x, my + start_y, text[ptr + k]);

            // Render to the right direction
            if(settings->direction == TEXT_HORIZONTAL) {
                mx += w + settings->cspacing;
            } else {
                my += charh;
            }
        }

        ptr += advance;
        line++;
    }
}
