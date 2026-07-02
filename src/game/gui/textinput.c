#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "controller/controller.h"
#include "game/gui/text/text.h"
#include "game/gui/textinput.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include "video/image.h"
#include "video/video.h"

#define COLOR_MENU_LINE 252
#define COLOR_MENU_BORDER 251
#define COLOR_MENU_BG 0

typedef struct textinput {
    int max_chars;
    size_t pos;
    int last_source;
    bool edit_by_default; ///< Start (and stay) in edit mode, for single-field screens

    bool bg_enabled;
    surface bg_surface;

    int text_max_lines;
    text_horizontal_align text_horizontal_align;
    font_size font_size;
    uint8_t text_shadow;
    vga_index text_shadow_color;
    text *text;
    str buf;
    str wheel_charset;

    textinput_filter_cb filter_cb;
    textinput_done_cb done_cb;
    void *userdata;
} textinput;

static void textinput_set_editing(component *c, bool editing) {
    if(c->editing != editing) {
        c->editing = editing;
        c->dirty = true;
    }
}

static bool textinput_wheel_active(component *c) {
    const textinput *ti = widget_get_obj(c);
    return ti->last_source == CTRL_TYPE_GAMEPAD && c->editing;
}

// This keeps a field from being left in edit mode.
static void textinput_focus(component *c, bool focused) {
    textinput *ti = widget_get_obj(c);
    textinput_set_editing(c, focused && ti->edit_by_default);
}

static void set_cursor(component *c, bool focused) {
    const textinput *ti = widget_get_obj(c);

    // Try to avoid pointless work
    if(!c->dirty) {
        return;
    }
    c->dirty = false;

    // Not focused, just show the text as-is (no cursor)
    if(!focused) {
        text_set_from_str(ti->text, &ti->buf);
        return;
    }

    // Focused, show the cursor
    str tmp;
    str_from(&tmp, &ti->buf);
    if(ti->last_source == CTRL_TYPE_GAMEPAD) {
        if(c->editing) {
            // If user is using a gamepad and has entered editing mode (via PUNCH), we show the cursor
            // ON TOP of the character.
            if(ti->pos >= str_size(&tmp)) {
                str_append_char(&tmp, CURSOR_CHAR);
            } else {
                str_set_at(&tmp, ti->pos, CURSOR_CHAR);
            }
        }
    } else {
        // In normal keyboard mode, we move the text around the cursor.
        str_insert_at(&tmp, ti->pos, CURSOR_CHAR);
    }
    text_set_from_str(ti->text, &tmp);
    str_free(&tmp);
}

static void refresh(component *c) {
    textinput *ti = widget_get_obj(c);
    str_truncate(&ti->buf, ti->max_chars - 1);
    c->dirty = true;
    text_set_from_str(ti->text, &ti->buf);
}

static void textinput_render(component *c) {
    const textinput *ti = widget_get_obj(c);
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

    // Render a black character on top of the cursor when using gamepad letter wheel
    if(component_is_selected(c) && !component_is_disabled(c) && textinput_wheel_active(c) &&
       ti->pos < str_size(&ti->buf)) {
        int16_t gx, gy;
        if(text_get_glyph_pos(ti->text, ti->pos, &gx, &gy)) {
            const font *font = fonts_get_font(text_get_font(ti->text));
            text_draw_glyph(font, str_at(&ti->buf, ti->pos), (int16_t)(c->x + left + gx), (int16_t)(c->y + top + gy),
                            0);
        }
    }
}

char textinput_wheel_step(const str *charset, char current, bool up) {
    size_t n = str_size(charset);
    if(n == 0) {
        return current;
    }
    size_t idx;
    if(!str_first_of(charset, current, &idx) && !str_first_of(charset, ' ', &idx)) {
        idx = 0;
    }
    idx = up ? (idx + 1) % n : (idx + n - 1) % n;
    return str_at(charset, idx);
}

static void textinput_wheel_scroll(component *c, bool up) {
    textinput *ti = widget_get_obj(c);
    const char new_char = textinput_wheel_step(&ti->wheel_charset, str_at(&ti->buf, ti->pos), up);
    if(ti->pos >= str_size(&ti->buf)) {
        str_append_char(&ti->buf, new_char);
    } else {
        str_set_at(&ti->buf, ti->pos, new_char);
    }
    refresh(c);
}

static void textinput_move_caret(component *c, bool right) {
    textinput *ti = widget_get_obj(c);
    if(right) {
        ti->pos = smin2(ti->pos + 1, str_size(&ti->buf));
    } else if(ti->pos > 0) {
        ti->pos--;
    }
    refresh(c);
}

static int textinput_action(component *c, int action, int source) {
    textinput *ti = widget_get_obj(c);
    // A connected but idle gamepad emits ACT_STOP every tick. That idle signal is
    // not real input, so it must not flip the field into gamepad mode while the
    // keyboard is being used.
    if(action != ACT_STOP && ti->last_source != source) {
        ti->last_source = source;
        c->dirty = true;
    }

    // Keyboard handling
    if(source != CTRL_TYPE_GAMEPAD) {
        switch(action) {
            case ACT_RIGHT:
                textinput_move_caret(c, true);
                return 0;
            case ACT_LEFT:
                textinput_move_caret(c, false);
                return 0;
            case ACT_PUNCH:
                if(ti->done_cb) {
                    ti->done_cb(c, ti->userdata);
                    return 0;
                }
                break;
            default:
                break;
        }
        return 1;
    }

    // If controller is gamepad and if not yet editing, enable edit mode.
    if(!c->editing) {
        if(action == ACT_PUNCH) {
            textinput_set_editing(c, true);
            return 0;
        }
        return 1;
    }

    // If controller is gamepad, and we are editing, work with the text.
    switch(action) {
        case ACT_RIGHT:
            textinput_move_caret(c, true);
            return 0;
        case ACT_LEFT:
            textinput_move_caret(c, false);
            return 0;
        case ACT_UP:
            textinput_wheel_scroll(c, true);
            return 0;
        case ACT_DOWN:
            textinput_wheel_scroll(c, false);
            return 0;
        case ACT_KICK:
            if(!ti->edit_by_default) {
                textinput_set_editing(c, false);
                return 0;
            }
            return 1;
        case ACT_PUNCH:
            if(ti->done_cb) {
                ti->done_cb(c, ti->userdata);
            }
            return 0;
        default:
            break;
    }
    return 1;
}

// '@' and '~' are not printable in this game
static bool is_valid_input(char c) {
    return isprint(c) && c != '@' && c != '~';
}

static int textinput_event(component *c, SDL_Event *e) {
    // Handle selection
    textinput *ti = widget_get_obj(c);
    if((e->type == SDL_TEXTINPUT || e->type == SDL_KEYDOWN) && ti->last_source != CTRL_TYPE_KEYBOARD) {
        ti->last_source = CTRL_TYPE_KEYBOARD;
        c->dirty = true;
    }
    // Only accept input if:
    // - The global filter accepts that this is text supported by font (is_valid_input)
    // - there is no text filter callback set OR the filter callback function accepts the input.
    if(e->type == SDL_TEXTINPUT && is_valid_input(e->text.text[0]) &&
       (ti->filter_cb == NULL || ti->filter_cb(e->text.text[0]))) {
        str_insert_at(&ti->buf, ti->pos, e->text.text[0]);
        str_truncate(&ti->buf, ti->max_chars - 1);
        ti->pos = smin2(ti->pos + 1, str_size(&ti->buf));
        refresh(c);
        return 0;
    } else if(e->type == SDL_KEYDOWN) {
        const unsigned char *state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_BACKSPACE]) {
            if(ti->pos > 0) {
                ti->pos--;
                str_delete_at(&ti->buf, ti->pos);
            }
            refresh(c);
        } else if(state[SDL_SCANCODE_DELETE]) {
            str_delete_at(&ti->buf, ti->pos);
            refresh(c);
        } else if(state[SDL_SCANCODE_V] && state[SDL_SCANCODE_LCTRL]) {
            if(SDL_HasClipboardText()) {
                char *clip = SDL_GetClipboardText();
                str filtered;
                str_create(&filtered);
                for(const char *p = clip; *p != '\0'; p++) {
                    if(is_valid_input(*p) && (ti->filter_cb == NULL || ti->filter_cb(*p))) {
                        str_append_char(&filtered, *p);
                    }
                }
                str_insert_buf_at(&ti->buf, ti->pos, str_c(&filtered), str_size(&filtered));
                str_truncate(&ti->buf, ti->max_chars - 1);
                ti->pos = smin2(ti->pos + str_size(&filtered), str_size(&ti->buf));
                str_free(&filtered);
                SDL_free(clip);
                refresh(c);
            }
        }
        return 0;
    }
    return 1;
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
    c->dirty = true;
    text_set_from_str(ti->text, &ti->buf);
    ti->pos = 0;
}

static void textinput_free(component *c) {
    textinput *ti = widget_get_obj(c);
    surface_free(&ti->bg_surface);
    text_free(&ti->text);
    str_free(&ti->buf);
    str_free(&ti->wheel_charset);
    omf_free(ti);
}

void textinput_enable_background(component *c, int enabled) {
    textinput *ti = widget_get_obj(c);
    ti->bg_enabled = enabled;
}

void textinput_set_filter_cb(component *c, textinput_filter_cb filter_cb) {
    textinput *ti = widget_get_obj(c);
    ti->filter_cb = filter_cb;
}

void textinput_set_done_cb(component *c, textinput_done_cb done_cb, void *userdata) {
    textinput *ti = widget_get_obj(c);
    ti->done_cb = done_cb;
    ti->userdata = userdata;
}

void textinput_set_text(component *c, char const *value) {
    textinput *ti = widget_get_obj(c);
    str_set_c(&ti->buf, value);
    refresh(c);
    ti->pos = str_size(&ti->buf);
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

void textinput_set_wheel_charset(component *c, const char *charset) {
    textinput *ti = widget_get_obj(c);
    str_set_c(&ti->wheel_charset, charset);
}

void textinput_set_edit_by_default(component *c, bool enabled) {
    textinput *ti = widget_get_obj(c);
    ti->edit_by_default = enabled;
    textinput_set_editing(c, enabled);
}

static void textinput_init(component *c, const gui_theme *theme) {
    textinput *ti = widget_get_obj(c);
    text_set_font(ti->text, ti->font_size != FONT_NONE ? ti->font_size : theme->text.font);
    text_set_color(ti->text, theme->text.primary_color);
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
    str_from_c(&ti->wheel_charset, "0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    ti->text_max_lines = 1;
    ti->bg_enabled = true;
    ti->max_chars = max_chars;
    ti->pos = 0;
    ti->font_size = FONT_SMALL;
    ti->text_horizontal_align = TEXT_ALIGN_CENTER;
    ti->text_shadow_color = 0;
    ti->text_shadow = GLYPH_SHADOW_NONE;
    ti->text = text_create();
    ti->pos = smin2(str_size(&ti->buf), (size_t)(ti->max_chars - 1));
    ti->last_source = CTRL_TYPE_KEYBOARD;

    component_set_help_text(c, help);

    // Widget stuff
    widget_set_obj(c, ti);
    widget_set_render_cb(c, textinput_render);
    widget_set_event_cb(c, textinput_event);
    widget_set_action_cb(c, textinput_action);
    widget_set_free_cb(c, textinput_free);
    widget_set_init_cb(c, textinput_init);
    widget_set_layout_cb(c, textinput_layout);
    widget_set_focus_cb(c, textinput_focus);
    return c;
}
