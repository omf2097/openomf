#include <stdlib.h>
#include <string.h>

#include "game/gui/progressbar.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include "video/image.h"
#include "video/surface.h"
#include "video/video.h"

const progressbar_theme _progressbar_theme_health = {
    .border_topleft_color = 0xB9,
    .border_bottomright_color = 0xBE,
    .bg_color = 0xF9,
    .bg_color_alt = 0xF9,
    .int_topleft_color = 0xB7,
    .int_bottomright_color = 0xB4,
    .int_bg_color = 0xF6,
};

const progressbar_theme _progressbar_theme_endurance = {
    .border_topleft_color = 0xB9,
    .border_bottomright_color = 0xBE,
    .bg_color = 0xF9,
    .bg_color_alt = 0xBE,
    .int_topleft_color = 0xE2,
    .int_bottomright_color = 0xE0,
    .int_bg_color = 0xF8,
};

const progressbar_theme _progressbar_theme_melee = {
    .border_topleft_color = 0xA2,
    .border_bottomright_color = 0xA2,
    .bg_color = 0,
    .bg_color_alt = 0,
    .int_topleft_color = 0xA7,
    .int_bottomright_color = 0xA3,
    .int_bg_color = 0xA5,
};

typedef struct progressbar {
    surface background;
    surface background_alt;
    surface fill_left;
    surface fill_mid;
    surface fill_right;
    bool surfaces_created;
    int orientation;
    int percentage;
    int display_percentage;
    progressbar_theme theme;
    int flashing;
    int rate;
    int state;
    int tick;
    bool highlight;
} progressbar;

void progressbar_set_progress(component *c, int percentage, bool animate) {
    progressbar *bar = widget_get_obj(c);
    bar->percentage = clamp(percentage, 0, 100);
    if(!animate || bar->percentage > bar->display_percentage) {
        // refilling the meter is instant
        bar->display_percentage = bar->percentage;
    }
}

void progressbar_set_flashing(component *c, int flashing, int rate) {
    progressbar *bar = widget_get_obj(c);
    if(flashing != bar->flashing) {
        bar->tick = 0;
        bar->state = 0;
    }
    bar->flashing = clamp(flashing, 0, 1);
    bar->rate = (rate < 0) ? 0 : rate;
}

void progressbar_set_highlight(component *c, bool highlight) {
    progressbar *bar = widget_get_obj(c);
    bar->highlight = highlight;
}

static void progressbar_render_fill(const progressbar *bar, int x, int y, int w, int h) {
    const int fill_w = w * bar->display_percentage / 100;
    const int fill_x = x + (bar->orientation == PROGRESSBAR_LEFT ? 0 : w - fill_w);
    const int offset = bar->highlight ? 1 : 0;

    // Start cap if its needed
    if(fill_w > 0) {
        video_draw_full(&bar->fill_left, fill_x, y, 1, h, 0, 0, offset, 255, 255, 0, 0);
    }
    // If we need to draw more than start and end caps, then horizontally draw & stretch the middle part.
    if(fill_w > 2) {
        video_draw_full(&bar->fill_mid, fill_x + 1, y, fill_w - 2, h, 0, 0, offset, 255, 255, 0, 0);
    }
    // End cap if its needed.
    if(fill_w > 1) {
        video_draw_full(&bar->fill_right, fill_x + fill_w - 1, y, 1, h, 0, 0, offset, 255, 255, 0, 0);
    }
}

static void progressbar_render(component *c) {
    const progressbar *bar = widget_get_obj(c);
    video_draw(bar->state ? &bar->background_alt : &bar->background, c->x, c->y);
    progressbar_render_fill(bar, c->x, c->y, c->w, c->h);
}

static void progressbar_tick(component *c) {
    progressbar *bar = widget_get_obj(c);
    if(bar->display_percentage > bar->percentage) {
        bar->display_percentage--;
    }
    if(bar->flashing) {
        if(bar->tick > bar->rate) {
            bar->tick = 0;
            bar->state = !bar->state;
        }
        bar->tick++;
    }
}

static void progressbar_free_surfaces(progressbar *bar) {
    if(bar->surfaces_created) {
        surface_free(&bar->background);
        surface_free(&bar->background_alt);
        surface_free(&bar->fill_left);
        surface_free(&bar->fill_mid);
        surface_free(&bar->fill_right);
        bar->surfaces_created = false;
    }
}

static void progressbar_free(component *c) {
    progressbar *bar = widget_get_obj(c);
    progressbar_free_surfaces(bar);
    omf_free(bar);
}

static void progressbar_create_background(surface *s, int w, int h, const progressbar_theme *t, uint8_t bg_color) {
    image img;
    image_create(&img, w, h);
    image_clear(&img, bg_color);
    image_rect_bevel(&img, 0, 0, w - 1, h - 1, t->border_topleft_color, t->border_bottomright_color,
                     t->border_bottomright_color, t->border_topleft_color);
    surface_create_from_image(s, &img);
    surface_set_transparency(s, 0);
    image_free(&img);
}

static void progressbar_create_column(surface *s, int h, uint8_t top, uint8_t middle, uint8_t bottom) {
    unsigned char *col = omf_malloc(h);
    memset(col, middle, h);
    col[0] = top;
    col[h - 1] = bottom;
    surface_create_from_data(s, 1, h, col);
    surface_set_transparency(s, -1);
    omf_free(col);
}

static void progressbar_layout(component *c, int x, int y, int w, int h) {
    progressbar *bar = widget_get_obj(c);
    const progressbar_theme *t = &bar->theme;

    progressbar_free_surfaces(bar);

    progressbar_create_background(&bar->background, w, h, t, t->bg_color);
    progressbar_create_background(&bar->background_alt, w, h, t, t->bg_color_alt);
    progressbar_create_column(&bar->fill_left, h, t->int_topleft_color, t->int_topleft_color, t->int_topleft_color);
    progressbar_create_column(&bar->fill_mid, h, t->int_topleft_color, t->int_bg_color, t->int_bottomright_color);
    progressbar_create_column(&bar->fill_right, h, t->int_bottomright_color, t->int_bottomright_color,
                              t->int_bottomright_color);
    bar->surfaces_created = true;
}

component *progressbar_create(progressbar_theme theme, int orientation, int percentage) {
    component *c = widget_create();
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    progressbar *local = omf_calloc(1, sizeof(progressbar));
    local->theme = theme;
    local->orientation = clamp(orientation, 0, 1);
    local->percentage = clamp(percentage, 0, 100);
    local->display_percentage = local->percentage;

    widget_set_obj(c, local);
    widget_set_render_cb(c, progressbar_render);
    widget_set_tick_cb(c, progressbar_tick);
    widget_set_free_cb(c, progressbar_free);
    widget_set_layout_cb(c, progressbar_layout);

    return c;
}
