#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "game/gui/textinput.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "video/color.h"
#include "video/image.h"
#include "video/video.h"

#define COLOR_MENU_LINE 252
#define COLOR_MENU_BORDER 251
#define COLOR_MENU_BG 0

typedef struct {
    char *text;
    text_settings tconf;
    int ticks;
    int dir;
    surface sur;
    char *buf;
    int max_chars;
    int pos;
    int bg_enabled;
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
            int tmp_s = text_width(&tb->tconf, tb->buf); // Total W minus last spacing
            int xspace = c->w - tb->tconf.padding.left - tb->tconf.padding.right;
            start_x += ceilf((xspace - tmp_s) / 2.0f);
            tb->tconf.halign = TEXT_LEFT;
            text_render(&tb->tconf, TEXT_DEFAULT, start_x + offset, c->y, c->w, c->h, "\x7F");
            tb->tconf.halign = TEXT_CENTER;
        } else {
            text_render(&tb->tconf, TEXT_DEFAULT, start_x + offset, c->y, c->w, c->h, "\x7F");
        }
    } else if(component_is_disabled(c)) {
        mode = TEXT_DISABLED;
    }

    text_render(&tb->tconf, mode, c->x, c->y, c->w, c->h, tb->buf);
}

static int textinput_action(component *c, int action) {
    textinput *tb = widget_get_obj(c);
    DEBUG("action %d", action);
    switch(action) {
        case ACT_RIGHT:
            if(!tb->buf[tb->pos]) {
                tb->buf[tb->pos] = ' ';
            }
            tb->pos = min2(tb->max_chars - 1, tb->pos + 1);
            if(!tb->buf[tb->pos]) {
                tb->buf[tb->pos] = ' ';
            }
            return 0;
            break;
        case ACT_LEFT:
            tb->pos = max2(0, tb->pos - 1);
            return 0;
            break;
        case ACT_UP:
            tb->buf[tb->pos] = tb->buf[tb->pos] + 1;
            if(tb->buf[tb->pos] > 126) {
                tb->buf[tb->pos] = 32;
            }
            return 0;
            break;
        case ACT_DOWN:
            tb->buf[tb->pos] = tb->buf[tb->pos] - 1;
            if(tb->buf[tb->pos] < 32) {
                tb->buf[tb->pos] = 126;
            }
            return 0;
            break;
    }
    return 1;
}

static int textinput_event(component *c, SDL_Event *e) {
    // Handle selection
    if(e->type == SDL_TEXTINPUT) {
        textinput *tb = widget_get_obj(c);
        tb->buf[tb->pos] = e->text.text[0];
        tb->pos = min2(tb->max_chars - 1, tb->pos + 1);
        // strncat(tb->buf, e->text.text, tb->max_chars - strlen(tb->buf));
        return 0;
    } else if(e->type == SDL_KEYDOWN) {
        textinput *tb = widget_get_obj(c);
        size_t len = strlen(tb->buf);
        const unsigned char *state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_BACKSPACE] || state[SDL_SCANCODE_DELETE]) {
            if(len > 1) {
                memmove(tb->buf + max2(tb->pos - 1, 0), tb->buf + max2(tb->pos, 1),
                        min2(tb->max_chars, tb->max_chars - tb->pos + 1));
                tb->pos = max2(0, tb->pos - 1);
            } else if(len == 1) {
                tb->buf[0] = 0;
                tb->pos = 0;
            }
        } else if(state[SDL_SCANCODE_V] && state[SDL_SCANCODE_LCTRL]) {
            if(SDL_HasClipboardText()) {
                char *clip = SDL_GetClipboardText();
                strncat(tb->buf + tb->pos, clip, tb->max_chars - tb->pos);
                tb->pos = min2(tb->max_chars, tb->pos + strlen(clip));
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

char *textinput_value(const component *c) {
    textinput *tb = widget_get_obj(c);
    // Trim trailing whitespace
    for(size_t i = strlen(tb->buf) - 1; i >= 0 && isspace(tb->buf[i]); --i) {
        tb->buf[i] = '\0';
    }
    return tb->buf;
}

static void textinput_free(component *c) {
    textinput *tb = widget_get_obj(c);
    surface_free(&tb->sur);
    omf_free(tb->text);
    omf_free(tb->buf);
    omf_free(tb);
}

void textinput_set_max_chars(component *c, int max_chars) {
    textinput *tb = widget_get_obj(c);
    tb->buf = omf_realloc(tb->buf, max_chars + 1);
    tb->buf[max_chars] = 0;

    tb->max_chars = max_chars;

    component_set_size_hints(c, text_char_width(&tb->tconf) * tb->max_chars, 10);
}

void textinput_enable_background(component *c, int enabled) {
    textinput *tb = widget_get_obj(c);
    tb->bg_enabled = enabled;
}

component *textinput_create(const text_settings *tconf, const char *text, const char *help, const char *initialvalue) {
    component *c = widget_create();

    textinput *tb = omf_calloc(1, sizeof(textinput));
    tb->text = strdup(text);
    memcpy(&tb->tconf, tconf, sizeof(text_settings));
    tb->bg_enabled = 1;
    tb->max_chars = 15;
    tb->pos = 0;

    component_set_help_text(c, help);

    // Background for field
    int tsize = text_char_width(&tb->tconf);
    image img;
    image_create(&img, 15 * tsize + 2, tsize + 3);
    image_clear(&img, COLOR_MENU_BG);
    image_rect(&img, 0, 0, 15 * tsize + 1, tsize + 2, COLOR_MENU_BORDER);
    surface_create_from_image(&tb->sur, &img);
    image_free(&img);

    // Copy over the initial value
    tb->buf = omf_calloc(tb->max_chars + 1, 1);
    strncpy(tb->buf, initialvalue, tb->max_chars);
    tb->pos = min2(strlen(initialvalue), tb->max_chars);

    component_set_size_hints(c, text_char_width(&tb->tconf) * tb->max_chars, 10);

    // Widget stuff
    widget_set_obj(c, tb);
    widget_set_render_cb(c, textinput_render);
    widget_set_event_cb(c, textinput_event);
    widget_set_action_cb(c, textinput_action);
    widget_set_tick_cb(c, textinput_tick);
    widget_set_free_cb(c, textinput_free);
    return c;
}
