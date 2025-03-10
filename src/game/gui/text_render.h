#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include <stdbool.h>
#include <stdint.h>

#include "resources/fonts.h"
#include "utils/str.h"

#define TEXT_DARK_GREEN 0xFE
#define TEXT_MEDIUM_GREEN 0xFE
#define TEXT_BLINKY_GREEN 0xFF
#define TEXT_BRIGHT_GREEN 0xFD
#define TEXT_TRN_BLUE 0xAB

typedef enum
{
    TEXT_TOP = 0,
    TEXT_MIDDLE,
    TEXT_BOTTOM
} text_valign;

typedef enum
{
    TEXT_LEFT = 0,
    TEXT_CENTER,
    TEXT_RIGHT
} text_halign;

typedef enum
{
    TEXT_DEFAULT = 0,
    TEXT_SELECTED,
    TEXT_UNSELECTED,
    TEXT_DISABLED
} text_mode;

typedef struct {
    uint8_t left;
    uint8_t right;
    uint8_t top;
    uint8_t bottom;
} text_padding;

typedef enum
{
    TEXT_HORIZONTAL = 0,
    TEXT_VERTICAL,
} text_direction;

enum
{
    TEXT_SHADOW_NONE = 0,
    TEXT_SHADOW_TOP = 0x1,
    TEXT_SHADOW_BOTTOM = 0x2,
    TEXT_SHADOW_LEFT = 0x4,
    TEXT_SHADOW_RIGHT = 0x8,
    TEXT_SHADOW_ALL = 0xF
};

typedef struct {
    uint8_t cforeground;
    uint8_t cdisabled;
    uint8_t cinactive;
    uint8_t cselected;
    uint8_t cshadow;
    text_valign valign;
    text_halign halign;
    text_padding padding;
    text_direction direction;
    font_size font;
    uint8_t shadow;
    uint8_t cspacing;
    uint8_t lspacing;
    bool strip_leading_whitespace : 1;
    bool strip_trailing_whitespace : 1;
    uint8_t max_lines;
} text_settings;

void text_defaults(text_settings *settings);
// only for testing
int text_find_max_strlen(const text_settings *settings, int max_chars, const char *ptr);

int text_find_line_count(const text_settings *settings, int cols, int rows, int len, const char *text,
                         int *longest_line_len);
int text_render_char(const text_settings *settings, vga_index color, int x, int y, char ch);
void text_render(const text_settings *settings, vga_index color, int x, int y, int w, int h, const char *text);
void text_render_str(const text_settings *settings, vga_index color, int x, int y, int w, int h, const str *text);
int text_char_width(font_size font);
int text_width(const text_settings *settings, const char *text);
int text_width_limit(const text_settings *settings, const char *text, int limit);

#endif // TEXT_RENDER_H
