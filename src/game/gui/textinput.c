#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "game/gui/text/text.h"
#include "game/gui/textinput.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "video/image.h"
#include "video/video.h"

#define COLOR_MENU_LINE 252
#define COLOR_MENU_BORDER 251
#define COLOR_MENU_BG 0

typedef struct textinput {
    int ticks;
    int max_chars;
    size_t pos;

    bool bg_enabled;
    surface bg_surface;

    int text_max_lines;
    text_horizontal_align text_horizontal_align;
    font_size font_size;
    uint8_t text_shadow;
    vga_index text_shadow_color;
    text *text;
    bool was_focused;
    str buf;

    textinput_done_cb done_cb;
    void *userdata;
} textinput;

static void set_cursor(component *c, bool focused) {
    textinput *ti = widget_get_obj(c);
    if(focused == ti->was_focused)
        return;
    if(focused) {
        str tmp;
        str_from(&tmp, &ti->buf);
        if(ti->pos == str_size(&tmp)) {
            str_append_c(&tmp, "\x7F");
        } else {
            str_set_at(&tmp, ti->pos, '\x7F');
        }
        text_set_from_str(ti->text, &tmp);
        str_free(&tmp);
    } else {
        text_set_from_str(ti->text, &ti->buf);
    }
    ti->was_focused = focused;
}

static void refresh(component *c) {
    textinput *ti = widget_get_obj(c);
    str_truncate(&ti->buf, ti->max_chars);
    ti->was_focused = false;
    text_set_from_str(ti->text, &ti->buf);
}

static void textinput_render(component *c) {
    textinput *ti = widget_get_obj(c);
    const gui_theme *theme = component_get_theme(c);

    if(ti->bg_enabled) {
        video_draw(&ti->bg_surface, c->x + 2, c->y);
    }

    if(component_is_selected(c)) {
        set_cursor(c, true);
        text_set_color(ti->text, theme->text.active_color);
    } else if(component_is_disabled(c)) {
        set_cursor(c, false);
        text_set_color(ti->text, theme->text.disabled_color);
    } else {
        set_cursor(c, false);
        text_set_color(ti->text, theme->text.inactive_color);
    }

    uint8_t left = 0, top = 0;
    if(ti->bg_enabled) {
        left += 2;
        top += 2;
    }
    text_draw(ti->text, c->x + left, c->y + top);
}

// Start from ' '. Support 0-9, ' ', and A-Z.
static char textinput_scroll_character(char cur, bool down) {
    if(cur == '\0') {
        cur = ' ';
    }

    char ret = down ? cur - 1 : cur + 1;
    if(ret == ' ' || (ret >= '0' && ret <= '9') || (ret >= 'A' && ret <= 'Z')) {
        return ret;
    }

    // In ASCII the order of the ranges is ' ', 0-9, and A-Z. As we are
    // starting from ' ', we reorder the ranges to be 0-9, ' ', and A-Z. This
    // makes both numbers and letters easier to find. The if conditions here
    // have to be specified by ASCII order from smallest to largest when going
    // down, and from largest to smallest when going up.
    if(down) {
        if(ret < ' ') {
            return '9';
        } else if(ret < '0') {
            return 'Z';
        } else if(ret < 'A') {
            return ' ';
        }
    } else {
        if(ret > 'Z') {
            return '0';
        } else if(ret > '9') {
            return ' ';
        } else if(ret > ' ') {
            return 'A';
        }
    }
    return ' ';
}

static int textinput_action(component *c, int action) {
    textinput *ti = widget_get_obj(c);
    char cursor_char = str_at(&ti->buf, ti->pos);
    char new_char;
    switch(action) {
        case ACT_RIGHT:
            if(cursor_char == '\0' && ti->pos >= str_size(&ti->buf)) {
                str_append_c(&ti->buf, " ");
            }
            ti->pos = min2(ti->max_chars - 1, ti->pos + 1);
            refresh(c);
            return 0;
        case ACT_LEFT:
            ti->pos = max2(0, ti->pos - 1);
            refresh(c);
            return 0;
        case ACT_UP:
            new_char = textinput_scroll_character(cursor_char, false);
            if(ti->pos >= str_size(&ti->buf)) {
                str_append_buf(&ti->buf, &new_char, 1);
            } else {
                str_set_at(&ti->buf, ti->pos, new_char);
            }
            refresh(c);
            return 0;
        case ACT_DOWN:
            new_char = textinput_scroll_character(cursor_char, true);
            if(ti->pos >= str_size(&ti->buf)) {
                str_append_buf(&ti->buf, &new_char, 1);
            } else {
                str_set_at(&ti->buf, ti->pos, new_char);
            }
            refresh(c);
            return 0;
        case ACT_PUNCH:
            if(ti->done_cb) {
                ti->done_cb(c, ti->userdata);
                return 0;
            }
            break;
    }
    return 1;
}

static bool is_valid_input(char c) {
    return (c >= 32 && c <= 126);
}

static int textinput_event(component *c, SDL_Event *e) {
    // Handle selection
    textinput *ti = widget_get_obj(c);
    if(e->type == SDL_TEXTINPUT && is_valid_input(e->text.text[0])) {
        str_insert_at(&ti->buf, ti->pos, e->text.text[0]);
        str_truncate(&ti->buf, ti->max_chars);
        ti->pos = min2(ti->max_chars - 1, ti->pos + 1);
        refresh(c);
        return 0;
    } else if(e->type == SDL_KEYDOWN) {
        const unsigned char *state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_BACKSPACE]) {
            ti->pos = max2(0, ti->pos - 1);
            str_delete_at(&ti->buf, ti->pos);
            refresh(c);
        } else if(state[SDL_SCANCODE_DELETE]) {
            if(str_delete_at(&ti->buf, ti->pos)) {
                ti->pos = max2(0, ti->pos);
            }
            refresh(c);
        } else if(state[SDL_SCANCODE_V] && state[SDL_SCANCODE_LCTRL]) {
            if(SDL_HasClipboardText()) {
                char *clip = SDL_GetClipboardText();
                str_insert_c_at(&ti->buf, ti->pos, clip);
                str_truncate(&ti->buf, ti->max_chars);
                ti->pos = min2(ti->max_chars - 1, ti->pos + strlen(clip));
                SDL_free(clip);
                refresh(c);
            }
        }
        return 0;
    }
    return 1;
}

static void textinput_tick(component *c) {
    textinput *ti = widget_get_obj(c);
    ti->ticks++;
}

const char *textinput_value(const component *c) {
    textinput *ti = widget_get_obj(c);
    str_strip(&ti->buf);
    ti->pos = 0;
    return str_c(&ti->buf);
}

void textinput_clear(component *c) {
    textinput *ti = widget_get_obj(c);
    str_truncate(&ti->buf, 0);
    text_set_from_str(ti->text, &ti->buf);
    ti->pos = 0;
}

static void textinput_free(component *c) {
    textinput *ti = widget_get_obj(c);
    surface_free(&ti->bg_surface);
    text_free(&ti->text);
    omf_free(ti);
}

void textinput_enable_background(component *c, int enabled) {
    textinput *ti = widget_get_obj(c);
    ti->bg_enabled = enabled;
}

void textinput_set_done_cb(component *c, textinput_done_cb done_cb, void *userdata) {
    textinput *ti = widget_get_obj(c);
    ti->done_cb = done_cb;
    ti->userdata = userdata;
}

void textinput_set_text(component *c, char const *value) {
    textinput *ti = widget_get_obj(c);
    str_set_c(&ti->buf, value);
    ti->pos = str_size(&ti->buf);
    refresh(c);
}

void textinput_set_font(component *c, font_size font) {
    textinput *ti = widget_get_obj(c);
    ti->font_size = font;
}

void textinput_set_horizontal_align(component *c, text_horizontal_align align) {
    textinput *ti = widget_get_obj(c);
    ti->text_horizontal_align = align;
}

void textinput_set_text_shadow(component *c, uint8_t shadow, vga_index color) {
    textinput *ti = widget_get_obj(c);
    ti->text_shadow = shadow;
    ti->text_shadow_color = color;
}

static void textinput_init(component *c, const gui_theme *theme) {
    textinput *ti = widget_get_obj(c);
    text_set_font(ti->text, ti->font_size != FONT_NONE ? ti->font_size : theme->text.font);
    text_set_color(ti->text, theme->text.primary_color);
    text_set_line_spacing(ti->text, 0);
    text_set_horizontal_align(ti->text, TEXT_ALIGN_LEFT);
    text_set_word_wrap(ti->text, false);
    text_set_shadow_style(ti->text, ti->text_shadow);
    text_set_shadow_color(ti->text, ti->text_shadow_color);
    text_set_margin(ti->text, (text_margin){0, 0, 0, 0});
    if(ti->bg_enabled) {
        text_set_margin(ti->text, (text_margin){1, 1, 1, 1});
    }
    refresh(c);
    if(c->h_hint < 0) {
        text_generate_layout(ti->text);
        int text_height = text_get_layout_height(ti->text) + (ti->bg_enabled ? 2 : 0);
        component_set_size_hints(c, c->w_hint, text_height);
    }
}

static void textinput_layout(component *c, int x, int y, int w, int h) {
    textinput *ti = widget_get_obj(c);
    text_set_horizontal_align(ti->text, ti->text_horizontal_align);
    if(ti->bg_enabled) {
        text_set_bounding_box(ti->text, w - 2, h - 2);
        image img;
        image_create(&img, w - 4, h);
        image_clear(&img, 0);
        image_rect(&img, 0, 0, w - 4, h, COLOR_MENU_BORDER);
        surface_create_from_image(&ti->bg_surface, &img);
        image_free(&img);
    } else {
        text_set_bounding_box(ti->text, w, h);
    }
    text_generate_layout(ti->text);
}

component *textinput_create(int max_chars, const char *help, const char *initial_value) {
    component *c = widget_create();

    textinput *ti = omf_calloc(1, sizeof(textinput));
    str_from_c(&ti->buf, initial_value);
    ti->text_max_lines = 1;
    ti->bg_enabled = true;
    ti->max_chars = max_chars;
    ti->pos = 0;
    ti->font_size = FONT_SMALL;
    ti->text_horizontal_align = TEXT_ALIGN_CENTER;
    ti->text_shadow_color = 0;
    ti->text_shadow = GLYPH_SHADOW_NONE;
    ti->text = text_create();
    ti->pos = min2(str_size(&ti->buf), ti->max_chars);

    component_set_help_text(c, help);

    // Widget stuff
    widget_set_obj(c, ti);
    widget_set_render_cb(c, textinput_render);
    widget_set_event_cb(c, textinput_event);
    widget_set_action_cb(c, textinput_action);
    widget_set_tick_cb(c, textinput_tick);
    widget_set_free_cb(c, textinput_free);
    widget_set_init_cb(c, textinput_init);
    widget_set_layout_cb(c, textinput_layout);
    return c;
}
