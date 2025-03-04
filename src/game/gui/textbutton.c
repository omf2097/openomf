#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "game/gui/menu_background.h"
#include "game/gui/text/text.h"
#include "game/gui/textbutton.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "video/video.h"

typedef struct textbutton {
    text *text;

    bool border_created;
    surface border;

    vga_index disabled_color;
    vga_index selected_color;
    vga_index default_color;

    textbutton_click_cb click_cb;
    void *userdata;
} textbutton;

void textbutton_set_border(component *c, vga_index color) {
    textbutton *tb = widget_get_obj(c);
    if(tb->border_created) {
        // destroy the old border first
        surface_free(&tb->border);
    }

    // create new border
    menu_background_border_create(&tb->border, text_get_layout_width(tb->text), text_get_layout_height(tb->text),
                                  color);
    tb->border_created = true;
}

void textbutton_set_text(component *c, const char *text) {
    textbutton *tb = widget_get_obj(c);
    text_set_from_c(tb->text, text);
    text_generate_layout(tb->text);
    component_set_size_hints(c, text_get_layout_width(tb->text), text_get_layout_height(tb->text));
}

static void textbutton_render(component *c) {
    textbutton *tb = widget_get_obj(c);

    // Make sure any changes are reflected in the text box.
    text_set_bounding_box(tb->text, c->w, c->h);
    if(component_is_selected(c)) {
        text_set_color(tb->text, tb->selected_color);
    } else if(component_is_disabled(c)) {
        text_set_color(tb->text, tb->disabled_color);
    } else {
        text_set_color(tb->text, tb->default_color);
    }
    // Border
    if(tb->border_created) {
        video_draw(&tb->border, c->x - 2, c->y - 2);
    }

    text_draw(tb->text, c->x, c->y);
}

void textbutton_set_default_color(component *c, vga_index color) {
    textbutton *tb = widget_get_obj(c);
    tb->default_color = color;
}

void textbutton_set_selected_color(component *c, vga_index color) {
    textbutton *tb = widget_get_obj(c);
    tb->selected_color = color;
}

void textbutton_set_disabled_color(component *c, vga_index color) {
    textbutton *tb = widget_get_obj(c);
    tb->disabled_color = color;
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

void textbutton_set_userdata(component *c, void *userdata) {
    textbutton *tb = widget_get_obj(c);
    tb->userdata = userdata;
}

static void textbutton_free(component *c) {
    textbutton *tb = widget_get_obj(c);
    if(tb->border_created) {
        surface_free(&tb->border);
    }
    text_free(&tb->text);
    omf_free(tb);
}

component *textbutton_create(const font_size font, const char *text, const char *help, int disabled,
                             textbutton_click_cb cb, void *userdata) {
    component *c = widget_create();
    component_disable(c, disabled);

    textbutton *tb = omf_calloc(1, sizeof(textbutton));
    tb->default_color = 0xFD;
    tb->disabled_color = 0xC0;
    tb->selected_color = 0xFF;

    tb->text = text_create_with_size(font, TEXT_BBOX_MAX, TEXT_BBOX_MAX);
    text_set_from_c(tb->text, text);
    text_set_horizontal_align(tb->text, ALIGN_TEXT_CENTER);
    text_set_vertical_align(tb->text, ALIGN_TEXT_MIDDLE);
    text_set_color(tb->text, tb->default_color);
    text_generate_layout(tb->text);

    component_set_size_hints(c, text_get_layout_width(tb->text), text_get_layout_height(tb->text));
    component_set_help_text(c, help);
    tb->click_cb = cb;
    tb->userdata = userdata;
    widget_set_obj(c, tb);

    widget_set_render_cb(c, textbutton_render);
    widget_set_action_cb(c, textbutton_action);
    widget_set_free_cb(c, textbutton_free);

    return c;
}
