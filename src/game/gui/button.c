#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "game/gui/button.h"
#include "game/gui/menu_background.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "video/video.h"

typedef struct button {
    char *text;
    text_settings tconf;

    bool border_created;
    surface border;

    button_click_cb click_cb;
    void *userdata;
} button;

void button_set_border(component *c, vga_index border_color) {
    button *tb = widget_get_obj(c);
    if(tb->border_created) {
        // destroy the old border first
        surface_free(&tb->border);
    }

    // create new border
    int fsize = text_char_width(&tb->tconf);
    int width = text_width(&tb->tconf, tb->text);
    menu_background_border_create(&tb->border, width + 6, fsize + 3, border_color);
    tb->border_created = 1;
}

void button_set_text(component *c, const char *text) {
    button *tb = widget_get_obj(c);
    if(tb->text) {
        omf_free(tb->text);
    }
    tb->text = omf_strdup(text);
    component_set_size_hints(c, text_width(&tb->tconf, text), 10);
}

static void button_render(component *c) {
    button *tb = widget_get_obj(c);

    // Select color and render
    int text_mode = TEXT_UNSELECTED;
    if(component_is_selected(c)) {
        text_mode = TEXT_SELECTED;
    } else if(component_is_disabled(c)) {
        text_mode = TEXT_DISABLED;
    }
    // Border
    if(tb->border_created) {
        video_draw(&tb->border, c->x - 2, c->y - 2);
    }

    text_render(&tb->tconf, text_mode, c->x, c->y, c->w, c->h, tb->text);
}

static int button_action(component *c, int action) {
    button *tb = widget_get_obj(c);

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

void button_set_userdata(component *c, void *userdata) {
    button *tb = widget_get_obj(c);
    tb->userdata = userdata;
}

static void button_free(component *c) {
    button *tb = widget_get_obj(c);
    if(tb->border_created) {
        surface_free(&tb->border);
    }
    omf_free(tb->text);
    omf_free(tb);
}

component *button_create(const text_settings *tconf, const char *text, const char *help, int disabled,
                         button_click_cb cb, void *userdata) {
    component *c = widget_create();
    component_disable(c, disabled);

    button *tb = omf_calloc(1, sizeof(button));
    tb->text = omf_strdup(text);
    component_set_size_hints(c, text_width(tconf, text), 8);
    component_set_help_text(c, help);
    memcpy(&tb->tconf, tconf, sizeof(text_settings));
    tb->click_cb = cb;
    tb->userdata = userdata;
    widget_set_obj(c, tb);

    widget_set_render_cb(c, button_render);
    widget_set_action_cb(c, button_action);
    widget_set_free_cb(c, button_free);

    return c;
}
