#include <ctype.h>
#include <math.h>

#include "game/gui/text_render.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/vector.h"
#include "video/video.h"

static unsigned char FIRST_PRINTABLE_CHAR = (unsigned char)' ';

static int text_chartoglyphindex(char c) {
    int ic = (int)(unsigned char)c;
    return ic - FIRST_PRINTABLE_CHAR;
}

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
    settings->strip_trailing_whitespace = true;
}

static inline surface *get_font_surface(const text_settings *settings, char ch) {
    // Make sure code is valid
    surface **ret;
    int code = text_chartoglyphindex(ch);
    if(code < 0) {
        return NULL;
    }
    switch(settings->font) {
        case FONT_BIG:
            ret = vector_get(&font_large.surfaces, code);
            break;
        case FONT_SMALL:
            ret = vector_get(&font_small.surfaces, code);
            break;
        case FONT_NET1:
            ret = vector_get(&font_net1.surfaces, code);
            break;
        case FONT_NET2:
            ret = vector_get(&font_net2.surfaces, code);
            break;
        default:
            ret = NULL;
            break;
    }
    return (ret == NULL) ? NULL : *ret;
}

static inline void render_char_shadow_surface(const text_settings *settings, const surface *sur, int x, int y) {
    if(settings->shadow & TEXT_SHADOW_RIGHT)
        video_draw_offset(sur, x + 1, y, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_LEFT)
        video_draw_offset(sur, x - 1, y, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_BOTTOM)
        video_draw_offset(sur, x, y + 1, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_TOP)
        video_draw_offset(sur, x, y - 1, settings->cshadow, 255);
}

static inline void render_char_surface(const text_settings *settings, text_mode state, surface *sur, int x, int y) {
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
    video_draw_offset(sur, x, y, color - 1, 255);
}

int text_render_char(const text_settings *settings, text_mode state, int x, int y, char ch) {
    surface *sur = get_font_surface(settings, ch);
    render_char_shadow_surface(settings, sur, x, y);
    render_char_surface(settings, state, sur, x, y);
    return sur->w;
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
    surface *sur = NULL;
    for(int i = 0; i < len; i++) {
        sur = get_font_surface(settings, text[i]);
        if(sur != NULL) {
            width += sur->w;
        }
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

int text_find_line_count(const text_settings *settings, int cols, int rows, int len, const char *text, int *longest) {
    int ptr = 0;
    int lines = 0;
    *longest = 0;
    while(lines < settings->max_lines && ptr < len) {
        // Find out how many characters for this row/col
        int line_len;
        if(settings->direction == TEXT_HORIZONTAL) {
            line_len = text_find_max_strlen(settings, cols, text + ptr);
        } else {
            line_len = text_find_max_strlen(settings, rows, text + ptr);
        }
        *longest = max2(*longest, line_len);

        ptr += line_len;
        lines++;
    }
    return lines;
}

static void text_render_len(const text_settings *settings, text_mode mode, int x, int y, int w, int h, const char *text,
                            int len) {
    int size = text_char_width(settings);
    int x_space = w - settings->padding.left - settings->padding.right;
    int y_space = h - settings->padding.top - settings->padding.bottom;
    int char_w = size + settings->cspacing;
    int char_h = size + settings->lspacing;
    int rows = (y_space + settings->lspacing) / char_h;
    int cols = (x_space + settings->cspacing) / char_w;
    int longest = 0;
    int fit_lines = text_find_line_count(settings, cols, rows, len, text, &longest);
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
            tmp_s = fit_lines * char_w - settings->cspacing; // Total W minus last spacing
            switch(settings->halign) {
                case TEXT_CENTER:
                    start_x += ceilf((x_space - tmp_s) / 2.0f);
                    break;
                case TEXT_RIGHT:
                    start_x += (x_space - tmp_s);
                    break;
                default:
                    break;
            }
            break;
        case TEXT_HORIZONTAL:
            tmp_s = fit_lines * char_h - settings->lspacing; // Total H minus last spacing
            switch(settings->valign) {
                case TEXT_MIDDLE:
                    start_y += floorf((y_space - tmp_s) / 2.0f);
                    break;
                case TEXT_BOTTOM:
                    start_y += (y_space - tmp_s);
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
                line_pw = line_len * char_w - settings->cspacing;
                my += char_h * line;

                // Horizontal alignment for this line
                switch(settings->halign) {
                    case TEXT_CENTER:
                        mx += floorf((x_space - line_pw) / 2.0f);
                        break;
                    case TEXT_RIGHT:
                        mx += (x_space - line_pw);
                        break;
                    default:
                        break;
                }
                break;
            case TEXT_VERTICAL:
                line_ph = line_len * char_h - settings->lspacing;
                mx += char_w * line;

                // Vertical alignment for this line
                switch(settings->valign) {
                    case TEXT_MIDDLE:
                        my += ceilf((y_space - line_ph) / 2.0f);
                        break;
                    case TEXT_BOTTOM:
                        my += (y_space - line_ph);
                        break;
                    default:
                        break;
                }
                break;
        }

        surface *sur;
        // Render characters
        int mxstart = mx;
        int mystart = my;
        int kstart = k;
        for(; k < line_len; k++) {
            // Skip line endings.
            if(text[ptr + k] == '\n')
                continue;

            // Render character shadow.
            sur = get_font_surface(settings, text[ptr + k]);
            if(sur == NULL) {
                continue;
            }

            render_char_shadow_surface(settings, sur, mx + start_x, my + start_y);

            // Render to the right direction
            if(settings->direction == TEXT_HORIZONTAL) {
                mx += sur->w + settings->cspacing;
            } else {
                my += char_h;
            }
        }

        mx = mxstart;
        my = mystart;
        k = kstart;
        for(; k < line_len; k++) {
            // Skip line endings.
            if(text[ptr + k] == '\n')
                continue;

            // Render character image.
            sur = get_font_surface(settings, text[ptr + k]);
            if(sur == NULL) {
                continue;
            }

            render_char_surface(settings, mode, sur, mx + start_x, my + start_y);

            // Render to the right direction
            if(settings->direction == TEXT_HORIZONTAL) {
                mx += sur->w + settings->cspacing;
            } else {
                my += char_h;
            }
        }

        ptr += advance;
        line++;
    }
}

void text_render(const text_settings *settings, text_mode mode, int x, int y, int w, int h, const char *text) {
    text_render_len(settings, mode, x, y, w, h, text, strlen(text));
}

void text_render_str(const text_settings *settings, text_mode mode, int x, int y, int w, int h, const str *text) {
    text_render_len(settings, mode, x, y, w, h, str_c(text), str_size(text));
}
