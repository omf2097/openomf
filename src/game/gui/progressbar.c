#include <stdlib.h>

#include "game/gui/progressbar.h"
#include "game/gui/widget.h"
#include "video/surface.h"
#include "video/image.h"
#include "video/video.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"

const progressbar_theme _progressbar_theme_health = {
    .border_topleft_color = {60,0,60,255},
    .border_bottomright_color = {178,0,223,255},
    .bg_color = {89,40,101,255},
    .bg_color_alt = {89,40,101,255},
    .int_topleft_color = {255,0,0,255},
    .int_bottomright_color = {158,0,0,255},
    .int_bg_color = {255,56,109,255},
};

const progressbar_theme _progressbar_theme_endurance = {
    .border_topleft_color = {60,0,60,255},
    .border_bottomright_color = {178,0,223,255},
    .bg_color = {89,40,101,255},
    .bg_color_alt = {178,0,223,255},
    .int_topleft_color = {24,117,138,255},
    .int_bottomright_color = {0,69,93,255},
    .int_bg_color = {97,150,186,255},
};

const progressbar_theme _progressbar_theme_melee = {
    .border_topleft_color = {0, 96, 0, 255},
    .border_bottomright_color = {0, 96, 0, 255},
    .bg_color = {80, 220, 80, 0},
    .bg_color_alt = {80, 220, 80, 0},
    .int_topleft_color = {0, 255, 0, 255},
    .int_bottomright_color = {0, 125, 0, 255},
    .int_bg_color = {0, 190, 0, 255},
};

typedef struct {
    surface *background;
    surface *background_alt;
    surface *block;
    int orientation;
    int percentage;
    progressbar_theme theme;
    int flashing;
    int rate;
    int state;
    int tick;
    int refresh;
} progressbar;

void progressbar_set_progress(component *c, int percentage) {
    progressbar *bar = widget_get_obj(c);
    int tmp = clamp(percentage, 0, 100);
    if(!bar->refresh)
        bar->refresh = (tmp != bar->percentage);
    bar->percentage = tmp;
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
    if(bar->refresh) {
        bar->refresh = 0;

        // Free old block first ...
        if(bar->block) {
            surface_free(bar->block);
        }

        // ... Then draw the new one
        float prog = bar->percentage / 100.0f;
        int w = c->w * prog;
        int h = c->h;
        if(w > 1 && h > 1) {
            image tmp;
            image_create(&tmp, w, h);
            image_clear(&tmp, bar->theme.int_bg_color);
            image_rect_bevel(&tmp,
                             0, 0,
                             w - 1, h - 1,
                             bar->theme.int_topleft_color,
                             bar->theme.int_bottomright_color,
                             bar->theme.int_bottomright_color,
                             bar->theme.int_topleft_color);
            if(bar->block == NULL) {
                bar->block = omf_calloc(1, sizeof(surface));
            }
            surface_create_from_image(bar->block, &tmp);
            surface_force_refresh(bar->block);
            image_free(&tmp);
        } else {
            omf_free(bar->block);
        }
    }

    // Render backgrond (flashing or not)
    if(bar->state) {
        video_render_sprite(bar->background_alt, c->x, c->y, BLEND_ALPHA, 0);
    } else {
        video_render_sprite(bar->background, c->x, c->y, BLEND_ALPHA, 0);
    }

    // Render block
    if(bar->block != NULL) {
        video_render_sprite(
            bar->block,
            c->x + (bar->orientation == PROGRESSBAR_LEFT ? 0 : c->w - bar->block->w + 1),
            c->y,
            BLEND_ALPHA,
            0);
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
    image_rect_bevel(&tmp,
                     0, 0, w-1, h-1,
                     bar->theme.border_topleft_color,
                     bar->theme.border_bottomright_color,
                     bar->theme.border_bottomright_color,
                     bar->theme.border_topleft_color);
    surface_create_from_image(bar->background, &tmp);
    image_free(&tmp);

    image_create(&tmp, w, h);
    image_clear(&tmp, bar->theme.bg_color_alt);
    image_rect_bevel(&tmp,
                     0, 0, w-1, h-1,
                     bar->theme.border_topleft_color,
                     bar->theme.border_bottomright_color,
                     bar->theme.border_bottomright_color,
                     bar->theme.border_topleft_color);
    surface_create_from_image(bar->background_alt, &tmp);
    image_free(&tmp);
}

component* progressbar_create(progressbar_theme theme, int orientation, int percentage) {
    component *c = widget_create();
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    progressbar *local = omf_calloc(1, sizeof(progressbar));
    local->theme = theme;
    local->orientation = clamp(orientation, 0, 1);
    local->percentage = clamp(percentage, 0, 100);
    local->refresh = 1;

    widget_set_obj(c, local);
    widget_set_render_cb(c, progressbar_render);
    widget_set_tick_cb(c, progressbar_tick);
    widget_set_free_cb(c, progressbar_free);
    widget_set_layout_cb(c, progressbar_layout);

    return c;
}
