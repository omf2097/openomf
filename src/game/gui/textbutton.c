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

    int border_created;
    uint8_t border_color;
    surface border;

    textbutton_click_cb click_cb;
    void *userdata;
} textbutton;

void textbutton_set_border(component *c, uint8_t color) {
    textbutton *tb = widget_get_obj(c);
    tb->border_color = color;
    if(tb->border_created) {
        // destroy the old border first
        surface_free(&tb->border);
    }

    // create new border
    int fsize = text_char_width(&tb->tconf);
    int width = text_width(&tb->tconf, tb->text);
    menu_background_border_create(&tb->border, width + 6, fsize + 3);
    tb->border_created = 1;
}

void textbutton_set_text(component *c, const char *text) {
    textbutton *tb = widget_get_obj(c);
    if(tb->text) {
        omf_free(tb->text);
    }
    tb->text = strdup(text);
    component_set_size_hints(c, text_width(&tb->tconf, text), 8);
}

static void textbutton_render(component *c) {
    textbutton *tb = widget_get_obj(c);

    // Select color and render
    int text_mode = TEXT_UNSELECTED;
    if(component_is_selected(c)) {
        text_mode = TEXT_SELECTED;
    } else if(component_is_disabled(c)) {
        text_mode = TEXT_DISABLED;
    }
    text_render(&tb->tconf, text_mode, c->x, c->y, c->w, c->h, tb->text);

    // Border
    if(tb->border_created) {
        video_draw(&tb->border, c->x - 2, c->y - 2);
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

static void textbutton_free(component *c) {
    textbutton *tb = widget_get_obj(c);
    if(tb->border_created) {
        surface_free(&tb->border);
    }
    omf_free(tb->text);
    omf_free(tb);
}

component *textbutton_create(const text_settings *tconf, const char *text, const char *help, int disabled,
                             textbutton_click_cb cb, void *userdata) {
    component *c = widget_create();
    component_disable(c, disabled);

    textbutton *tb = omf_calloc(1, sizeof(textbutton));
    tb->text = strdup(text);
    component_set_size_hints(c, text_width(tconf, text), 8);
    component_set_help_text(c, help);
    memcpy(&tb->tconf, tconf, sizeof(text_settings));
    tb->click_cb = cb;
    tb->userdata = userdata;
    widget_set_obj(c, tb);

    widget_set_render_cb(c, textbutton_render);
    widget_set_action_cb(c, textbutton_action);
    widget_set_free_cb(c, textbutton_free);

    return c;
}
