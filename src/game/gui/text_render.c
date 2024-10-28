#include <ctype.h>
#include <math.h>

#include "game/gui/text_render.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/vector.h"
#include "video/video.h"
#include "utils/allocator.h"

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


int text_render_char_uncached(const text_settings *settings, text_mode state, int x, int y, char ch, bool shadow) {
    surface *sur = get_font_surface(settings, ch);

    if(sur == NULL) {
        return 0;
    }
    // Handle shadows if necessary
    if (shadow != false) {
        if(settings->shadow & TEXT_SHADOW_RIGHT)
            video_draw_offset(sur, x + 1, y, settings->cshadow, 255);
        if(settings->shadow & TEXT_SHADOW_LEFT)
            video_draw_offset(sur, x - 1, y, settings->cshadow, 255);
        if(settings->shadow & TEXT_SHADOW_BOTTOM)
            video_draw_offset(sur, x, y + 1, settings->cshadow, 255);
        if(settings->shadow & TEXT_SHADOW_TOP)
            video_draw_offset(sur, x, y - 1, settings->cshadow, 255);

        return sur->w;
    }

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
    video_draw_offset(sur, x, y, color - 1, 255);
    return sur->w;
}

/*static inline void render_char_shadow_surface(const text_settings *settings, const surface *sur, int x, int y) {
    if(settings->shadow & TEXT_SHADOW_RIGHT)
        video_draw_offset(sur, x + 1, y, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_LEFT)
        video_draw_offset(sur, x - 1, y, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_BOTTOM)
        video_draw_offset(sur, x, y + 1, settings->cshadow, 255);
    if(settings->shadow & TEXT_SHADOW_TOP)
        video_draw_offset(sur, x, y - 1, settings->cshadow, 255);
}*/

/*static inline void render_char_surface(const text_settings *settings, text_mode state, surface *sur, int x, int y) {
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
}*/

/*int text_render_char(const text_settings *settings, text_mode state, int x, int y, char ch) {
    surface *sur = get_font_surface(settings, ch);
    render_char_shadow_surface(settings, sur, x, y);
    render_char_surface(settings, state, sur, x, y);
    return sur->w;
}*/

void letter_set_parameters(text_object *cached_text, surface *sur, int x, int y, int offset, int limit)
{
    cached_text->cur_letter->x = x;
    cached_text->cur_letter->y = y;
    cached_text->cur_letter->sur = sur;
    cached_text->cur_letter->offset = offset;
    cached_text->cur_letter->limit = limit;
    cached_text->cur_letter++;
    cached_text->letter_count++;
}

int text_render_char(text_object *cached_text, const text_settings *settings, text_mode state, int x, int y, char ch, bool shadow) {
    // Make sure code is valid
    int code = ch - 32;
    surface **sur = NULL;
    if(code < 0) {
        return 0;
    }

    // Select font face surface
    if(settings->font == FONT_BIG) {
        sur = vector_get(&font_large.surfaces, code);
    } else if(settings->font == FONT_SMALL) {
        sur = vector_get(&font_small.surfaces, code);
    } else if(settings->font == FONT_NET1) {
        sur = vector_get(&font_net1.surfaces, code);
    } else if(settings->font == FONT_NET2) {
        sur = vector_get(&font_net2.surfaces, code);
    }

    if(sur == NULL || *sur == NULL) {
        return 0;
    }

    // Empty character's don't need to be rendered just skip the space.
    if (ch == ' ') {
        return (*sur)->w;
    }
 
    // Handle shadows if necessary
    if (shadow != false) {
        if(settings->shadow & TEXT_SHADOW_RIGHT)
            letter_set_parameters(cached_text, *sur, x + 1, y, settings->cshadow, 255);
        if(settings->shadow & TEXT_SHADOW_LEFT)
            letter_set_parameters(cached_text, *sur, x - 1, y, settings->cshadow, 255);
        if(settings->shadow & TEXT_SHADOW_BOTTOM)
            letter_set_parameters(cached_text, *sur, x, y + 1, settings->cshadow, 255);
        if(settings->shadow & TEXT_SHADOW_TOP)
            letter_set_parameters(cached_text, *sur, x, y - 1, settings->cshadow, 255);

        return (*sur)->w;
    }

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
    letter_set_parameters(cached_text, *sur, x, y, color - 1, 255);
    return (*sur)->w;
}

// TODO: Make this function return a char* instead of an int.
int text_find_max_strlen(const text_settings *settings, int max_chars, const char *ptr) {
    int i = 0;

    // Skip whitespace at the start of the string
    if(settings->strip_leading_whitespace) {
        while ((*ptr != 0) && (*ptr == ' ')) {
            ptr++;
            i++;
        }

        if(*ptr == 0) {
            return i;
        }
    }

    // Walk through the rest of the string
    int max = max_chars + i;
    int breakpoint = max;
    int next_breakpoint = breakpoint;
    while((*ptr != 0) && (*ptr != '\n')) {
        if(*ptr == ' ') {
            next_breakpoint = i + 1;
        } else {
            breakpoint = next_breakpoint;
        }

        if(i >= max) {
            return breakpoint;
        }
        ptr++;
        i++;
    }

    // If we detect a newline, this line ends here.
    if (*ptr == '\n') {
        return i + 1;
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

int text_find_line_count(const text_settings *settings, int cols, int rows, const char *text, int *longest) {
    const char* ptr = text;
    int lines = 0;
    int space;
    if(settings->direction == TEXT_HORIZONTAL) {
        space = cols;
    } else {
        space = rows;
    }

    while(*ptr != 0) {
        // Find out how many characters for this row/col
        int length = text_find_max_strlen(settings, space, ptr);
        ptr += length;
        *longest = max2(*longest, length);
        lines++;
    }
    return lines;
}

/*static void text_render_len(const text_settings *settings, text_mode mode, int x, int y, int w, int h, const char *text,
                            int len) {
    int size = text_char_width(settings);
    int x_space = w - settings->padding.left - settings->padding.right;
    int y_space = h - settings->padding.top - settings->padding.bottom;
    int char_w = size + settings->cspacing;
    int char_h = size + settings->lspacing;
    int rows = (y_space + settings->lspacing) / char_h;
    int cols = (x_space + settings->cspacing) / char_w;
    int longest = 0;
    int fit_lines = text_find_line_count(settings, cols, rows, text, &longest);
    int max_chars = settings->direction == TEXT_HORIZONTAL ? cols : rows;
    if(max_chars == 0) {
        DEBUG("Warning: Text has zero size! text: '%s'");
        max_chars = 1;
    }
}*/

void text_object_invalidate_cache(text_object *obj)
{
    obj->dirty = true;
}

// This function should not be called outside of the text_render.c because it is heavy and may cause significant performance drops
// on slow systems.
static void setup_letter_locations(text_object *cached_text, const text_settings *settings, text_mode mode, int x, int y, int w, int h, const char *text) {
    int size = text_char_width(settings);
    int x_space = w - settings->padding.left - settings->padding.right;
    int y_space = h - settings->padding.top - settings->padding.bottom;
    int char_w = size + settings->cspacing;
    int char_h = size + settings->lspacing;
    int rows = (y_space + settings->lspacing) / char_h;
    int cols = (x_space + settings->cspacing) / char_w;
    int longest = 0;
    int fit_lines = text_find_line_count(settings, cols, rows, text, &longest);

    int start_x = x + settings->padding.left;
    int start_y = y + settings->padding.top;
    int tmp_s = 0;

    // Initial alignment for whole text block
    switch(settings->direction) {
        case TEXT_VERTICAL:
            tmp_s = fit_lines * char_w - settings->cspacing; // Total W minus last spacing
            switch(settings->halign) {
                case TEXT_CENTER:
                    start_x += ((x_space - tmp_s) + 1) / 2;
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
                    start_y += (y_space - tmp_s) / 2;
                    break;
                case TEXT_BOTTOM:
                    start_y += (y_space - tmp_s);
                    break;
                default:
                    break;
            }
            break;
    }

    const char* ptr = text;
    int line = 0;
    int space;
    if(settings->direction == TEXT_HORIZONTAL) {
        space = cols;
    } else {
        space = rows;
    }

    while((*ptr != 0) && (line < fit_lines)) {
        int line_len;
        int mx = 0;
        int my = 0;
        int line_pw;
        int line_ph;
        const char* prevptr = ptr;
        const char* line_end;
        // Find out how many characters for this row/col
        line_len = text_find_max_strlen(settings, space, ptr);
        line_end = ptr + line_len;

        // Skip spaces
        if(settings->strip_leading_whitespace) {
            while((*ptr != 0) && (*ptr == ' ') && (line_len > 0)) {
                ptr++;
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
                        mx += (x_space - line_pw) / 2;
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
                        my += ((y_space - line_ph) + 1) / 2;
                        break;
                    case TEXT_BOTTOM:
                        my += (y_space - line_ph);
                        break;
                    default:
                        break;
                }
                break;
        }

        int mxstart = mx;
        int mystart = my;
        const char* shadow_ptr = ptr;

        // Render shadow characters.
        while((*shadow_ptr != 0) && (shadow_ptr < line_end)) {
            // Skip line endings.
            if (*shadow_ptr == '\n') {
                shadow_ptr++;
                continue;
            }

            // Render character
            w = text_render_char(cached_text, settings, mode, mx + start_x, my + start_y, *shadow_ptr, true);
            shadow_ptr++;

            // Render to the right direction
            if(settings->direction == TEXT_HORIZONTAL) {
                mx += w + settings->cspacing;
            } else {
                my += char_h;
            }
        }

        // Render regular color characters.
        mx = mxstart;
        my = mystart;
        while ((*ptr != 0) && (ptr < line_end)) {
            // Skip line endings.
            if(*ptr == '\n') {
                ptr++;
                continue;
            }

            // Render character
            w = text_render_char(cached_text, settings, mode, mx + start_x, my + start_y, *ptr, false);
            ptr++;

            // Render to the right direction
            if(settings->direction == TEXT_HORIZONTAL) {
                mx += w + settings->cspacing;
            } else {
                my += char_h;
            }
        }

        ptr = prevptr + line_len;
        line++;
    }
}

void text_objects_free(text_object *cached_text, size_t count)
{
    for (size_t i = 0; i < count; i += 1) {
        omf_free(cached_text[i].letters);
    }
}

void text_objects_invalidate(text_object *text_cache, size_t cache_size)
{
    for (size_t i = 0; i < cache_size; i++) {
        text_cache[i].dirty = true;
    }
}

void text_render(text_object *cached_text, const text_settings *settings, text_mode mode, int x, int y, int w, int h, const char *text)
{
    bool dirty = false;
    dirty |= (cached_text->x != x);
    dirty |= (cached_text->y != y);
    if (w > 0) {
        dirty |= (cached_text->w != w);
    }
    if (h > 0) {
        dirty |= (cached_text->h != h);
    }
    dirty |= (cached_text->text != text);
    dirty |= (cached_text->mode != mode);
    dirty |= (memcmp(&cached_text->settings, settings, sizeof(cached_text->settings)) != 0);

    if (cached_text->dirty || dirty) {
        size_t text_len = strlen(text);
        if (text_len == 0) {
            return;
        }

        if (cached_text->max_letters < (text_len * 4)) {
            size_t size = (text_len * sizeof(letter) * 4);
            //assertf(size < (1024 * 40), "%i %s", size, text); // Allow up to 40KB of cache per text block. Support up to 3 shadows per char.
            cached_text->letters = omf_realloc(cached_text->letters, size);
            cached_text->max_letters = text_len * 4;
        }

        // Reset the letter pointer back to the first chached letter.
        cached_text->cur_letter = cached_text->letters;
        cached_text->letter_count = 0;
        // Calculate the screen offsets for the letters.
        setup_letter_locations(cached_text, settings, mode, x, y, w, h, text);
        cached_text->x = x;
        cached_text->y = y;
        cached_text->w = w;
        cached_text->h = h;
        cached_text->text = text;
        cached_text->mode = mode;
        memcpy(&cached_text->settings, settings, sizeof(cached_text->settings));
        cached_text->dirty = true;
        // This is kind of annoying.. h and w can be less than zero to specify a "figure it out" value.
        if ((h <= 0) || (w <= 0)) {
            int height = 0;
            int width = 0;
            for (uint32_t i = 0; i < cached_text->letter_count; i += 1) {
                const letter* letterptr = &(cached_text->letters[i]);
                int calculated_height = (letterptr->y + letterptr->sur->h) - y;
                if (calculated_height > height) {
                    height = calculated_height;
                }

                int calculated_width = (letterptr->x + letterptr->sur->w) - x;
                if (calculated_width > width) {
                    width = calculated_width;
                }
            }

            cached_text->h = height;
            cached_text->w = width;
        }

#ifdef DEBUGMODE
        // Validate all letterptrs are non null
        for (unsigned int i = 0; i < cached_text->letter_count; i += 1) {
            const letter* letterptr = &(cached_text->letters[i]);
            assert(letterptr != NULL);//, "%i", i);
        }
#endif

        //assertf((cached_text->max_letters > cached_text->letter_count), "Max:%u < Count:%u TextLen:%i %s", (unsigned int)cached_text->max_letters, (unsigned int)cached_text->letter_count, text_len, text);
        //assertf(cached_text->h > 0, "%s", text);
        //assertf(cached_text->w > 0, "%s", text);
        //assertf(cached_text->x < 320, "%s", text);
        //assertf(cached_text->y < 240, "%s", text);
        //assertf(cached_text->x > 0, "%i %s", cached_text->x, text);
        //assertf(cached_text->y > 0, "%i %s", cached_text->y, text);
    }

    video_render_text_block(cached_text);
    cached_text->dirty = false;
}

/*void text_render_str(const text_settings *settings, text_mode mode, int x, int y, int w, int h, const str *text) {
    text_render_len(settings, mode, x, y, w, h, str_c(text), str_size(text));
}*/

