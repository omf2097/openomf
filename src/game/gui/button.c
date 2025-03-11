#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio.h"
#include "game/gui/button.h"
#include "game/gui/menu_background.h"
#include "game/gui/text/text.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "video/video.h"

typedef struct button {
    text *text;
    bool use_border;
    surface border;
    button_click_cb click_cb;
    void *userdata;
} button;

void button_set_text(component *c, const char *text) {
    button *b = widget_get_obj(c);
    text_set_from_c(b->text, text);
    text_generate_layout(b->text);
    component_set_size_hints(c, text_get_layout_width(b->text), text_get_layout_height(b->text));
}

static void button_render(component *c) {
    button *b = widget_get_obj(c);
    const gui_theme *theme = component_get_theme(c);

    text_set_color(b->text, theme->text.primary_color);
    if(component_is_selected(c)) {
        text_set_color(b->text, theme->text.active_color);
    } else if(component_is_disabled(c)) {
        text_set_color(b->text, theme->text.disabled_color);
    }

    if(b->use_border) {
        video_draw(&b->border, c->x - 2, c->y - 2);
    }
    text_draw(b->text, c->x, c->y);
}

static int button_action(component *c, int action) {
    button *b = widget_get_obj(c);
    if(action == ACT_KICK || action == ACT_PUNCH) {
        if(b->click_cb) {
            b->click_cb(c, b->userdata);
        }
        audio_play_sound(20, 0.5f, 0.0f, 2.0f);
        return 0;
    }
    return 1;
}

void button_set_userdata(component *c, void *userdata) {
    button *b = widget_get_obj(c);
    b->userdata = userdata;
}

static void button_free(component *c) {
    button *b = widget_get_obj(c);
    if(b->use_border) {
        surface_free(&b->border);
    }
    text_free(&b->text);
    omf_free(b);
}

/*
 * Figure out the size we want for the button box.
 */
static void button_init(component *c, const gui_theme *theme) {
    button *b = widget_get_obj(c);
    text_set_font(b->text, theme->text.font);
    text_set_color(b->text, theme->text.primary_color);
    text_set_line_spacing(b->text, 0);
    text_generate_layout(b->text);

    int text_width = text_get_layout_width(b->text);
    int text_height = text_get_layout_height(b->text);
    if(b->use_border) {
        component_set_size_hints(c, text_width + 6, text_height + 3);
    } else {
        component_set_size_hints(c, text_width, text_height);
    }
}

/**
 * Prerender the border box and produce the final text object (with correct bounding box)
 */
static void button_layout(component *c, int x, int y, int w, int h) {
    const gui_theme *theme = component_get_theme(c);
    button *b = widget_get_obj(c);
    text_set_bounding_box(b->text, w, h);
    text_set_horizontal_align(b->text, ALIGN_TEXT_CENTER);
    text_generate_layout(b->text);
    if(b->use_border) {
        menu_background_border_create(&b->border, w, h, theme->dialog.border_color);
    }
}

component *button_create(const char *text, const char *help, bool disabled, bool border, button_click_cb cb,
                         void *userdata) {
    component *c = widget_create();
    component_disable(c, disabled);
    component_set_help_text(c, help);

    button *b = omf_calloc(1, sizeof(button));
    b->text = text_create_with_size(FONT_BIG, TEXT_BBOX_MAX, TEXT_BBOX_MAX);
    text_set_from_c(b->text, text);
    b->use_border = border;
    b->click_cb = cb;
    b->userdata = userdata;
    widget_set_obj(c, b);

    widget_set_render_cb(c, button_render);
    widget_set_action_cb(c, button_action);
    widget_set_free_cb(c, button_free);
    widget_set_init_cb(c, button_init);
    widget_set_layout_cb(c, button_layout);
    return c;
}
