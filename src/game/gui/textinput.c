#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game/gui/textinput.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/compat.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "video/color.h"
#include "video/image.h"
#include "video/video.h"

#define COLOR_MENU_LINE color_create(0, 0, 89, 255)
#define COLOR_MENU_BORDER color_create(0, 0, 243, 255)
#define COLOR_MENU_BG color_create(4, 4, 16, 210)

typedef struct {
    char *text;
    text_settings tconf;
    int ticks;
    int dir;
    surface sur;
    char *buf;
    int max_chars;
    int bg_enabled;
} textinput;

static void textinput_render(component *c) {
    textinput *tb = widget_get_obj(c);
    int chars = strlen(tb->buf);

    if(tb->bg_enabled) {
        video_render_sprite(&tb->sur, c->x + (c->w - tb->sur.w) / 2, c->y - 2, BLEND_ALPHA, 0);
    }

    if(component_is_selected(c)) {
        if(chars > 0) {
            tb->tconf.cforeground = color_create(80, 220, 80, 255);
            tb->buf[chars] = '\x7F';
            tb->buf[chars + 1] = 0;
            text_render(&tb->tconf, c->x, c->y, c->w, c->h, tb->buf);
            tb->buf[chars] = 0;
        }
    } else if(component_is_disabled(c)) {
        if(chars > 0) {
            tb->tconf.cforeground = color_create(121, 121, 121, 255);
            text_render(&tb->tconf, c->x, c->y, c->w, c->h, tb->buf);
        }
    } else {
        if(chars > 0) {
            tb->tconf.cforeground = color_create(0, 121, 0, 255);
            text_render(&tb->tconf, c->x, c->y, c->w, c->h, tb->buf);
        }
    }
    if(chars == 0) {
        tb->tconf.cforeground = color_create(121, 121, 121, 255);
        text_render(&tb->tconf, c->x, c->y, c->w, c->h, tb->text);
    }
}

static int textinput_event(component *c, SDL_Event *e) {
    // Handle selection
    if(e->type == SDL_TEXTINPUT) {
        textinput *tb = widget_get_obj(c);
        strncat(tb->buf, e->text.text, tb->max_chars - strlen(tb->buf));
        return 0;
    } else if(e->type == SDL_KEYDOWN) {
        textinput *tb = widget_get_obj(c);
        size_t len = strlen(tb->buf);
        const unsigned char *state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_BACKSPACE] || state[SDL_SCANCODE_DELETE]) {
            if(len > 0) {
                tb->buf[len - 1] = '\0';
            }
        } else if(state[SDL_SCANCODE_LEFT]) {
            // TODO move cursor to the left
        } else if(state[SDL_SCANCODE_RIGHT]) {
            // TODO move cursor to the right
        } else if(state[SDL_SCANCODE_V] && state[SDL_SCANCODE_LCTRL]) {
            if(SDL_HasClipboardText()) {
                strncat(tb->buf, SDL_GetClipboardText(), tb->max_chars - strlen(tb->buf));
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
}

void textinput_enable_background(component *c, int enabled) {
    textinput *tb = widget_get_obj(c);
    tb->bg_enabled = enabled;
}

component *textinput_create(const text_settings *tconf, const char *text, const char *initialvalue) {
    component *c = widget_create();

    textinput *tb = omf_calloc(1, sizeof(textinput));
    tb->text = strdup(text);
    memcpy(&tb->tconf, tconf, sizeof(text_settings));
    tb->bg_enabled = 1;
    tb->max_chars = 15;

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

    // Widget stuff
    widget_set_obj(c, tb);
    widget_set_render_cb(c, textinput_render);
    widget_set_event_cb(c, textinput_event);
    widget_set_tick_cb(c, textinput_tick);
    widget_set_free_cb(c, textinput_free);
    return c;
}
