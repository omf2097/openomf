#include <stdlib.h>

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

typedef struct {
    surface *background;
    surface *background_alt;
    surface *block;
    int orientation;
    int percentage;
    int display_percentage;
    progressbar_theme theme;
    int flashing;
    int rate;
    int state;
    int tick;
    int refresh;
} progressbar;

void progressbar_set_progress(component *c, int percentage, bool animate) {
    progressbar *bar = widget_get_obj(c);
    int tmp = clamp(percentage, 0, 100);
    if(!bar->refresh)
        bar->refresh = (tmp != bar->percentage);
    bar->percentage = tmp;
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

static void progressbar_render(component *c) {
    progressbar *bar = widget_get_obj(c);

    // If necessary, refresh the progress block
    if(bar->refresh || bar->display_percentage > bar->percentage) {
        bar->refresh = 0;

        if(bar->display_percentage > bar->percentage) {
            bar->display_percentage--;
        }

        // Free old block first ...
        if(bar->block) {
            surface_free(bar->block);
        }

        // ... Then draw the new one
        float prog = bar->display_percentage / 100.0f;
        int w = c->w * prog;
        int h = c->h;
        if(w > 1 && h > 1) {
            image tmp;
            image_create(&tmp, w, h);
            image_clear(&tmp, bar->theme.int_bg_color);
            image_rect_bevel(&tmp, 0, 0, w - 1, h - 1, bar->theme.int_topleft_color, bar->theme.int_bottomright_color,
                             bar->theme.int_bottomright_color, bar->theme.int_topleft_color);
            if(bar->block == NULL) {
                bar->block = omf_calloc(1, sizeof(surface));
            }
            surface_create_from_image(bar->block, &tmp);
            image_free(&tmp);
        } else {
            omf_free(bar->block);
        }
    }

    // Render background (flashing or not)
    if(bar->state) {
        video_draw(bar->background_alt, c->x, c->y);
    } else {
        video_draw(bar->background, c->x, c->y);
    }

    // Render block
    if(bar->block != NULL) {
        video_draw(bar->block, c->x + (bar->orientation == PROGRESSBAR_LEFT ? 0 : c->w - bar->block->w), c->y);
    }
}

static void progressbar_tick(component *c) {
    progressbar *bar = widget_get_obj(c);
    if(bar->flashing) {
        if(bar->tick > bar->rate) {
            bar->tick = 0;
            bar->state = !bar->state;
        }
        bar->tick++;
    }
}

static void progressbar_free(component *c) {
    progressbar *bar = widget_get_obj(c);
    if(bar->block) {
        surface_free(bar->block);
        omf_free(bar->block);
    }
    surface_free(bar->background);
    omf_free(bar->background);
    surface_free(bar->background_alt);
    omf_free(bar->background_alt);
    omf_free(bar);
}

static void progressbar_layout(component *c, int x, int y, int w, int h) {
    image tmp;
    progressbar *bar = widget_get_obj(c);

    // Allocate everything
    bar->background = omf_calloc(1, sizeof(surface));
    bar->background_alt = omf_calloc(1, sizeof(surface));
    bar->block = NULL;

    // Background,
    image_create(&tmp, w, h);
    image_clear(&tmp, bar->theme.bg_color);
    image_rect_bevel(&tmp, 0, 0, w - 1, h - 1, bar->theme.border_topleft_color, bar->theme.border_bottomright_color,
                     bar->theme.border_bottomright_color, bar->theme.border_topleft_color);
    surface_create_from_image(bar->background, &tmp);
    image_free(&tmp);

    image_create(&tmp, w, h);
    image_clear(&tmp, bar->theme.bg_color_alt);
    image_rect_bevel(&tmp, 0, 0, w - 1, h - 1, bar->theme.border_topleft_color, bar->theme.border_bottomright_color,
                     bar->theme.border_bottomright_color, bar->theme.border_topleft_color);
    surface_create_from_image(bar->background_alt, &tmp);
    image_free(&tmp);
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
    local->refresh = 1;

    widget_set_obj(c, local);
    widget_set_render_cb(c, progressbar_render);
    widget_set_tick_cb(c, progressbar_tick);
    widget_set_free_cb(c, progressbar_free);
    widget_set_layout_cb(c, progressbar_layout);

    return c;
}
