#include <stdlib.h>
#include <string.h>

#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text/text.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "video/video.h"

typedef struct spritebutton {
    text *text;
    const surface *img;
    int active_ticks;

    int16_t override_color;
    text_vertical_align vertical_align;
    text_horizontal_align horizontal_align;
    text_row_direction row_direction;
    font_size font;
    text_margin margins;

    spritebutton_click_cb click_cb;
    spritebutton_tick_cb tick_cb;
    spritebutton_focus_cb focus_cb;
    bool free_userdata;
    void *userdata;
} spritebutton;

static void spritebutton_render(component *c) {
    spritebutton *b = widget_get_obj(c);
    const gui_theme *theme = component_get_theme(c);

    if(component_is_disabled(c)) {
        video_draw_offset(b->img, c->x, c->y, 5, 0x5F);
    } else if(b->active_ticks != 0) {
        video_draw(b->img, c->x, c->y);
    }

    if(b->text != NULL) {
        text_set_color(b->text, theme->text.primary_color);
        if(b->override_color > -1) {
            text_set_color(b->text, b->override_color);
        } else if(component_is_disabled(c)) {
            text_set_color(b->text, theme->text.disabled_color);
        } else if(b->active_ticks > 0) {
            text_set_color(b->text, theme->text.active_color);
        }
        text_draw(b->text, c->x, c->y);
    }
}

static void spritebutton_free(component *c) {
    spritebutton *b = widget_get_obj(c);
    if(b->free_userdata) {
        omf_free(b->userdata);
    }
    text_free(&b->text);
    omf_free(b);
}

static void spritebutton_tick(component *c) {
    spritebutton *b = widget_get_obj(c);
    if(b->active_ticks > 0) {
        b->active_ticks--;
    }
    if(b->tick_cb) {
        b->tick_cb(c, b->userdata);
    }
}

static void spritebutton_focus(component *c, bool focused) {
    spritebutton *b = widget_get_obj(c);
    if(b->focus_cb) {
        b->focus_cb(c, focused, b->userdata);
    }
}

static int spritebutton_action(component *c, int action) {
    spritebutton *b = widget_get_obj(c);
    if(component_is_disabled(c)) {
        return 1;
    }
    if(action == ACT_KICK || action == ACT_PUNCH) {
        if(b->active_ticks >= 0) {
            b->active_ticks = 10;
        }
        if(b->click_cb) {
            b->click_cb(c, b->userdata);
        }
        return 0;
    }
    return 1;
}

static void spritebutton_layout(component *c, int x, int y, int w, int h) {
    const gui_theme *theme = component_get_theme(c);
    spritebutton *b = widget_get_obj(c);
    if(b->text != NULL) {
        text_set_font(b->text, b->font != FONT_NONE ? b->font : theme->text.font);
        text_set_color(b->text, b->override_color > -1 ? b->override_color : theme->text.primary_color);
        text_set_bounding_box(b->text, w, h);
        text_set_horizontal_align(b->text, b->horizontal_align);
        text_set_vertical_align(b->text, b->vertical_align);
        text_set_direction(b->text, b->row_direction);
        text_set_margin(b->text, b->margins);
        text_generate_layout(b->text);
    }
}

component *spritebutton_create(const char *text, const surface *img, bool disabled, spritebutton_click_cb cb,
                               void *userdata) {
    component *c = widget_create();
    component_disable(c, disabled);
    component_set_supports(c, true, true, true);
    component_set_size_hints(c, img->w, img->h);

    spritebutton *b = omf_calloc(1, sizeof(spritebutton));
    b->text = (text != NULL) ? text_create_from_c(text) : NULL;
    b->click_cb = cb;
    b->vertical_align = TEXT_ALIGN_MIDDLE;
    b->horizontal_align = TEXT_ALIGN_CENTER;
    b->row_direction = TEXT_ROW_HORIZONTAL;
    b->margins = (text_margin){0, 0, 0, 0};
    b->font = FONT_NONE;
    b->img = img;
    b->override_color = -1;
    b->active_ticks = 0;
    b->userdata = userdata;
    b->tick_cb = NULL;
    b->focus_cb = NULL;
    widget_set_obj(c, b);

    widget_set_render_cb(c, spritebutton_render);
    widget_set_action_cb(c, spritebutton_action);
    widget_set_focus_cb(c, spritebutton_focus);
    widget_set_tick_cb(c, spritebutton_tick);
    widget_set_free_cb(c, spritebutton_free);
    widget_set_layout_cb(c, spritebutton_layout);
    return c;
}

void spritebutton_set_horizontal_align(component *c, text_horizontal_align align) {
    spritebutton *b = widget_get_obj(c);
    b->horizontal_align = align;
}

void spritebutton_set_vertical_align(component *c, text_vertical_align align) {
    spritebutton *b = widget_get_obj(c);
    b->vertical_align = align;
}

void spritebutton_set_text_direction(component *c, text_row_direction direction) {
    spritebutton *b = widget_get_obj(c);
    b->row_direction = direction;
}

void spritebutton_set_font(component *c, font_size font) {
    spritebutton *b = widget_get_obj(c);
    b->font = font;
}

void spritebutton_set_text_color(component *c, vga_index color) {
    spritebutton *b = widget_get_obj(c);
    b->override_color = color;
}

void spritebutton_set_text_margin(component *c, text_margin margins) {
    spritebutton *b = widget_get_obj(c);
    b->margins = margins;
}

void spritebutton_set_tick_cb(component *c, spritebutton_tick_cb cb) {
    spritebutton *b = widget_get_obj(c);
    b->tick_cb = cb;
}

void spritebutton_set_focus_cb(component *c, spritebutton_focus_cb cb) {
    spritebutton *b = widget_get_obj(c);
    b->focus_cb = cb;
}

void spritebutton_set_always_display(component *c) {
    spritebutton *b = widget_get_obj(c);
    b->active_ticks = -1;
}

void spritebutton_set_free_userdata(component *c, bool free_userdata) {
    spritebutton *b = widget_get_obj(c);
    b->free_userdata = free_userdata;
}
