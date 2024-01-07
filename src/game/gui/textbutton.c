#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "game/gui/menu_background.h"
#include "game/gui/textbutton.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "video/video.h"

typedef struct {
    char *text;
    text_settings tconf;
    int ticks;
    int dir;

    int border_enabled;
    int border_created;
    color border_color;
    surface border;

    textbutton_click_cb click_cb;
    void *userdata;
} textbutton;

void textbutton_set_border(component *c, color col) {
    textbutton *tb = widget_get_obj(c);
    tb->border_enabled = 1;
    tb->border_color = col;
    if(tb->border_created) {
        // destroy the old border first
        surface_free(&tb->border);
    }

    // create new border
    int chars = strlen(tb->text);
    int fsize = text_char_width(&tb->tconf);
    int width = chars * fsize;
    menu_background_border_create(&tb->border, width + 6, fsize + 3);
    tb->border_created = 1;
}

void textbutton_remove_border(component *c) {
    textbutton *tb = widget_get_obj(c);
    tb->border_enabled = 0;
}

void textbutton_set_text(component *c, const char *text) {
    textbutton *tb = widget_get_obj(c);
    if(tb->text) {
        omf_free(tb->text);
    }
    tb->text = strdup(text);
}

static void textbutton_render(component *c) {
    textbutton *tb = widget_get_obj(c);

    // Select color and render
    if(component_is_selected(c)) {
        int t = tb->ticks / 2;
        tb->tconf.cforeground = color_create(80 - t, 220 - t * 2, 80 - t, 255);
    } else if(component_is_disabled(c)) {
        tb->tconf.cforeground = color_create(121, 121, 121, 255);
    } else {
        tb->tconf.cforeground = color_create(0, 121, 0, 255);
    }
    text_render(&tb->tconf, c->x, c->y, c->w, c->h, tb->text);

    // Border
    if(tb->border_enabled) {
        video_render_sprite(&tb->border, c->x - 2, c->y - 2, BLEND_ALPHA, 0);
    }
}

static int textbutton_action(component *c, int action) {
    textbutton *tb = widget_get_obj(c);

    // Handle selection
    if(action == ACT_KICK || action == ACT_PUNCH) {
        if(tb->click_cb) {
            tb->click_cb(c, tb->userdata);
        }
        audio_play_sound(20, 0.5f, 0.0f, 2.0f);
        return 0;
    }
    return 1;
}

static void textbutton_tick(component *c) {
    textbutton *tb = widget_get_obj(c);
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

static void textbutton_free(component *c) {
    textbutton *tb = widget_get_obj(c);

    if(tb->border_created) {
        surface_free(&tb->border);
    }
    omf_free(tb->text);
    omf_free(tb);
}

component *textbutton_create(const text_settings *tconf, const char *text, int disabled, textbutton_click_cb cb,
                             void *userdata) {
    component *c = widget_create();
    component_disable(c, disabled);

    textbutton *tb = omf_calloc(1, sizeof(textbutton));
    tb->text = strdup(text);
    memcpy(&tb->tconf, tconf, sizeof(text_settings));
    tb->click_cb = cb;
    tb->userdata = userdata;
    widget_set_obj(c, tb);

    widget_set_render_cb(c, textbutton_render);
    widget_set_action_cb(c, textbutton_action);
    widget_set_tick_cb(c, textbutton_tick);
    widget_set_free_cb(c, textbutton_free);

    return c;
}
