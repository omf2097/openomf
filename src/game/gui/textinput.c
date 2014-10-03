#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "game/gui/textinput.h"
#include "game/gui/widget.h"
#include "utils/vector.h"
#include "video/video.h"
#include "video/color.h"
#include "video/image.h"
#include "utils/log.h"
#include "utils/compat.h"

#define COLOR_MENU_LINE   color_create(0,0,89,255)
#define COLOR_MENU_BORDER color_create(0,0,243,255)
#define COLOR_MENU_BG     color_create(4,4,16,210)

typedef struct {
    char *text;
    const font *font;
    int ticks;
    int dir;
    int pos_;
    int *pos;
    surface sur;
    char buf[50];
} textinput;

static void textinput_render(component *c) {
    textinput *tb = widget_get_obj(c);
    /*char buf[100];*/
    int chars;
    int width;
    int xoff;
    chars = strlen(tb->buf);
    width = 15*tb->font->w;
    xoff = (c->w - width)/2;
    video_render_sprite(&tb->sur, c->x + xoff-2, c->y -2, BLEND_ALPHA, 0);
    if(component_is_selected(c)) {
        int t = tb->ticks / 2;
        if (chars > 0) {
            font_render(tb->font, tb->buf, c->x + xoff, c->y, color_create(80, 220, 80, 255));
        }
        font_render(tb->font, "", c->x+ xoff + chars * tb->font->w, c->y, color_create(121 - t, 121 - t, 121 - t, 255));
    } else if (component_is_disabled(c)) {
        if (chars > 0) {
            font_render(tb->font, tb->buf, c->x + xoff, c->y, color_create(121, 121, 121, 255));
        }
    } else {
        if (chars > 0) {
            font_render(tb->font, tb->buf, c->x + xoff, c->y, color_create(0, 121, 0, 255));
        }
    }
    if (chars == 0) {
        font_render(tb->font, tb->text, c->x + xoff, c->y, color_create(121, 121, 121, 255));
    }
}

static int textinput_event(component *c, SDL_Event *e) {
    // Handle selection
    if (e->type == SDL_TEXTINPUT) {
        textinput *tb = widget_get_obj(c);
        size_t len = strlen(tb->buf);
        if (strlen(e->text.text) == 1) {
            // make sure it is not a unicode sequence
            unsigned char c = e->text.text[0];
            if (c >= 32 && c <= 126) {
                // only allow ASCII through
                if (len < sizeof(tb->buf)-1) {
                    tb->buf[len+1] = '\0';
                    tb->buf[len] = c;
                }
            }
        }
    } else if (e->type == SDL_KEYDOWN) {
        textinput *tb = widget_get_obj(c);
        size_t len = strlen(tb->buf);
        const unsigned char *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_BACKSPACE] || state[SDL_SCANCODE_DELETE]) {
            if (len > 0) {
                tb->buf[len-1] = '\0';
            }
        } else if(state[SDL_SCANCODE_LEFT]) {
            // TODO move cursor to the left
        } else if(state[SDL_SCANCODE_RIGHT]) {
            // TODO move cursor to the right
        } else if(state[SDL_SCANCODE_V] && state[SDL_SCANCODE_LCTRL]) {
            if(SDL_HasClipboardText()) {
                char* clip_text = SDL_GetClipboardText();
                int c_size = strlen(clip_text);
                if((c_size + len) > sizeof(tb->buf)-1) {
                    c_size = sizeof(tb->buf) - 1 - len;
                }
                memcpy(tb->buf + len, clip_text, c_size);
                len += c_size;
                tb->buf[len] = 0;
            }
        }
    }
    return 1;
}

void textinput_focus(component *c, int focus) {
    if(focus) {
        SDL_StartTextInput();
    } else {
        SDL_StopTextInput();
    }
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

char* textinput_value(const component *c) {
    textinput *tb = widget_get_obj(c);
    return tb->buf;
}

static void textinput_free(component *c) {
    textinput *tb = widget_get_obj(c);
    surface_free(&tb->sur);
    free(tb->text);
    free(tb);
}

component* textinput_create(const font *font, const char *text, const char *initialvalue) {
    component *c = widget_create();

    textinput *tb = malloc(sizeof(textinput));
    memset(tb, 0, sizeof(textinput));
    tb->text = strdup(text);
    tb->font = font;
    tb->pos = &tb->pos_;

    // Background for field
    image img;
    image_create(&img, 15*font->w+2, font->h+3);
    image_clear(&img, COLOR_MENU_BG);
    image_rect(&img, 0, 0, 15*font->w+1, font->h+2, COLOR_MENU_BORDER);
    surface_create_from_image(&tb->sur, &img);
    image_free(&img);

    // Copy over the initial value
    memcpy(tb->buf, initialvalue, strlen(initialvalue)+1);

    // Widget stuff
    widget_set_obj(c, tb);
    widget_set_render_cb(c, textinput_render);
    widget_set_event_cb(c, textinput_event);
    widget_set_tick_cb(c, textinput_tick);
    widget_set_free_cb(c, textinput_free);
    return c;
}
