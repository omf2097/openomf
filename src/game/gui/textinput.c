#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
    text_settings tconf;
    int ticks;
    int dir;
    surface sur;
    str text;
    int max_chars;
    int pos;
    int bg_enabled;

    textinput_done_cb done_cb;
    void *userdata;
    text_object text_cache[2];
} textinput;

static void textinput_render(component *c) {
    textinput *tb = widget_get_obj(c);

    if(tb->bg_enabled) {
        video_draw(&tb->sur, c->x + (c->w - tb->sur.w) / 2, c->y - 2);
    }

    text_mode mode = TEXT_UNSELECTED;
    if(component_is_selected(c)) {
        mode = TEXT_SELECTED;
        int i = (tb->ticks / 10) % 16;
        if(i > 8) {
            i = 16 - i;
        }
        tb->tconf.cforeground = 216 + i;
        int offset = tb->pos * text_char_width(&tb->tconf);

        int start_x = c->x + tb->tconf.padding.left;
        if(tb->tconf.halign == TEXT_CENTER) {
            int tmp_s = text_width(&tb->tconf, str_c(&tb->text)); // Total W minus last spacing
            int xspace = c->w - tb->tconf.padding.left - tb->tconf.padding.right;
            start_x += ceilf((xspace - tmp_s) / 2.0f);
            tb->tconf.halign = TEXT_LEFT;
            text_render(&tb->text_cache[0], &tb->tconf, TEXT_DEFAULT, start_x + offset, c->y, c->w, c->h, "\x7F");
            tb->tconf.halign = TEXT_CENTER;
        } else {
            text_render(&tb->text_cache[0], &tb->tconf, TEXT_DEFAULT, start_x + offset, c->y, c->w, c->h, "\x7F");
        }
    } else if(component_is_disabled(c)) {
        mode = TEXT_DISABLED;
    }

    text_render(&tb->text_cache[1], &tb->tconf, mode, c->x, c->y, c->w, c->h, str_c(&tb->text));
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
    textinput *tb = widget_get_obj(c);
    char cursor_char = str_at(&tb->text, tb->pos);
    char new_char;
    switch(action) {
        case ACT_RIGHT:
            if(cursor_char == '\0' && tb->pos >= (int)str_size(&tb->text)) {
                str_append_c(&tb->text, " ");
            }
            tb->pos = min2(tb->max_chars - 1, tb->pos + 1);
            str_truncate(&tb->text, tb->max_chars);
            return 0;
            break;
        case ACT_LEFT:
            tb->pos = max2(0, tb->pos - 1);
            return 0;
            break;
        case ACT_UP:
            new_char = textinput_scroll_character(cursor_char, false);
            if(tb->pos >= (int)str_size(&tb->text)) {
                str_append_buf(&tb->text, &new_char, 1);
            } else {
                str_set_at(&tb->text, tb->pos, new_char);
            }
            str_truncate(&tb->text, tb->max_chars);
            return 0;
            break;
        case ACT_DOWN:
            new_char = textinput_scroll_character(cursor_char, true);
            if(tb->pos >= (int)str_size(&tb->text)) {
                str_append_buf(&tb->text, &new_char, 1);
            } else {
                str_set_at(&tb->text, tb->pos, new_char);
            }
            str_truncate(&tb->text, tb->max_chars);
            return 0;
            break;
        case ACT_PUNCH:
            if(tb->done_cb) {
                tb->done_cb(c, tb->userdata);
                return 0;
            }
            break;
    }
    return 1;
}

static int textinput_event(component *c, SDL_Event *e) {
    // Handle selection
    if(e->type == SDL_TEXTINPUT) {
        textinput *tb = widget_get_obj(c);
        str_insert_at(&tb->text, tb->pos, e->text.text[0]);
        str_truncate(&tb->text, tb->max_chars);
        tb->pos = min2(tb->max_chars - 1, tb->pos + 1);
        return 0;
    } else if(e->type == SDL_KEYDOWN) {
        textinput *tb = widget_get_obj(c);
        const unsigned char *state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_BACKSPACE]) {
            tb->pos = max2(0, tb->pos - 1);
            str_delete_at(&tb->text, tb->pos);
        } else if(state[SDL_SCANCODE_DELETE]) {
            if(str_delete_at(&tb->text, tb->pos)) {
                tb->pos = max2(0, tb->pos);
            }
        } else if(state[SDL_SCANCODE_V] && state[SDL_SCANCODE_LCTRL]) {
            if(SDL_HasClipboardText()) {
                char *clip = SDL_GetClipboardText();
                str_insert_c_at(&tb->text, tb->pos, clip);
                str_truncate(&tb->text, tb->max_chars);
                tb->pos = min2(tb->max_chars - 1, tb->pos + strlen(clip));
                SDL_free(clip);
            }
        }
        return 0;
    }
    return 1;
}

static void textinput_tick(component *c) {
    textinput *tb = widget_get_obj(c);
    if(!tb->dir) {
        tb->ticks++;
    } else {
        tb->ticks--;
    }
    if(tb->ticks > 120) {
        tb->dir = 1;
    }
    if(tb->ticks == 0) {
        tb->dir = 0;
    }
}

const char *textinput_value(const component *c) {
    textinput *tb = widget_get_obj(c);
    // Trim whitespace
    str_strip(&tb->text);
    tb->pos = 0;
    return str_c(&tb->text);
}

void textinput_clear(component *c) {
    textinput *tb = widget_get_obj(c);
    str_truncate(&tb->text, 0);
    tb->pos = 0;
}

static void textinput_free(component *c) {
    textinput *tb = widget_get_obj(c);
    surface_free(&tb->sur);
    str_free(&tb->text);
    text_objects_free(tb->text_cache, (sizeof(tb->text_cache) / sizeof(tb->text_cache[0])));
    omf_free(tb);
}

void textinput_enable_background(component *c, int enabled) {
    textinput *tb = widget_get_obj(c);
    tb->bg_enabled = enabled;
}

void textinput_set_done_cb(component *c, textinput_done_cb done_cb, void *userdata) {
    textinput *tb = widget_get_obj(c);
    tb->done_cb = done_cb;
    tb->userdata = userdata;
}

component *textinput_create(const text_settings *tconf, int max_chars, const char *help, const char *initialvalue) {
    component *c = widget_create();

    textinput *tb = omf_calloc(1, sizeof(textinput));
    memcpy(&tb->tconf, tconf, sizeof(text_settings));
    tb->tconf.max_lines = 1;
    tb->bg_enabled = 1;
    tb->max_chars = max_chars;
    tb->pos = 0;

    component_set_help_text(c, help);

    // Background for field
    int tsize = text_char_width(&tb->tconf);
    image img;
    image_create(&img, tb->max_chars * tsize + 2, tsize + 3);
    image_clear(&img, COLOR_MENU_BG);
    image_rect(&img, 0, 0, tb->max_chars * tsize + 1, tsize + 2, COLOR_MENU_BORDER);
    surface_create_from_image(&tb->sur, &img);
    image_free(&img);

    // Copy over the initial value
    str_from_c(&tb->text, initialvalue);
    str_truncate(&tb->text, tb->max_chars);
    tb->pos = min2(str_size(&tb->text), tb->max_chars);

    component_set_size_hints(c, text_char_width(&tb->tconf) * tb->max_chars, 10);
    component_set_size_hints(c, tb->max_chars * tsize + 2, tsize + 3);

    // Widget stuff
    widget_set_obj(c, tb);
    widget_set_render_cb(c, textinput_render);
    widget_set_event_cb(c, textinput_event);
    widget_set_action_cb(c, textinput_action);
    widget_set_tick_cb(c, textinput_tick);
    widget_set_free_cb(c, textinput_free);
    tb->text_cache[0].dynamic = true;
    tb->text_cache[1].dynamic = true;
    return c;
}
