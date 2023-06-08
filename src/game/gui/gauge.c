#include "game/gui/gauge.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include "video/surface.h"
#include "video/video.h"

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char data[24]; // 24 is the biggest pixel_image we make
} pixel_image;

// Palette indices used:
// 0, 255, 0 = 0xA7
// 0, 223, 0 = 0xA6
// 0, 190, 0 = 0xA5
// 0, 157, 0 = 0xA4
// 0, 125, 0 = 0xA3
// 0, 93,  0 = 0xA2
// 0, 60,  0 = 0xA1
// 0, 32,  0 = 0xA0

// clang-format off
static const pixel_image gauge_small_off_img = {
    .width = 3,
    .height = 3,
    .data = {
        0xA3, 0xA2, 0xA1,
        0xA2, 0xA1, 0xA0,
        0xA1, 0xA0, 0xA0
    }
};

static const pixel_image gauge_small_on_img = {
    .width = 3,
    .height = 3,
    .data = {
        0xA7, 0xA6, 0xA5,
        0xA5, 0xA4, 0xA3,
        0xA3, 0xA2, 0xA1
    }
};

static const pixel_image gauge_big_on_img = {
    .width = 8,
    .height = 3,
    .data = {
        0xA7, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA5,
        0xA5, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA3,
        0xA3, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA1
    }
};

static const pixel_image gauge_big_off_img = {
    .width = 8,
    .height = 3,
    .data = {
        0xA3, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA1,
        0xA2, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA0,
        0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA0
    }
};

// clang-format on

// Local small gauge type
typedef struct {
    int size;
    int lit;
    gauge_type type;
    surface on;
    surface off;
} gauge;

static void surface_from_pix_img(surface *sur, const pixel_image *pix) {
    surface_create_from_data(sur, SURFACE_TYPE_PALETTE, pix->width, pix->height, pix->data);
}

static void gauge_render(component *c) {
    gauge *g = widget_get_obj(c);

    int k = 0;
    int x = c->x;
    for(; k < g->lit; k++) {
        video_draw(&g->on, x, c->y);
        x += g->on.w;
    }
    for(; k < g->size; k++) {
        video_draw(&g->off, x, c->y);
        x += g->on.w;
    }
}

static void gauge_free(component *c) {
    gauge *g = widget_get_obj(c);
    surface_free(&g->on);
    surface_free(&g->off);
    omf_free(g);
}

void gauge_set_lit(component *c, int lit) {
    gauge *g = widget_get_obj(c);
    if(lit != g->lit) {
        g->lit = lit;
    }
}

int gauge_get_lit(component *c) {
    gauge *g = widget_get_obj(c);
    return g->lit;
}

int gauge_get_size(component *c) {
    gauge *g = widget_get_obj(c);
    return g->size;
}

void gauge_set_size(component *c, int size) {
    gauge *g = widget_get_obj(c);
    if(size != g->size) {
        g->size = size;
        if(g->lit > g->size) {
            g->lit = g->size;
        }
    }
}

component *gauge_create(gauge_type type, int size, int lit) {
    component *c = widget_create();
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    // Local information
    gauge *local = omf_calloc(1, sizeof(gauge));
    local->size = size;
    local->type = type;
    local->lit = clamp(lit, 0, size);

    // Initialize surfaces
    if(local->type == GAUGE_SMALL) {
        surface_from_pix_img(&local->on, &gauge_small_on_img);
        surface_from_pix_img(&local->off, &gauge_small_off_img);
        component_set_size_hints(c, gauge_small_on_img.width * size, gauge_small_on_img.height);
    } else {
        surface_from_pix_img(&local->on, &gauge_big_on_img);
        surface_from_pix_img(&local->off, &gauge_big_off_img);
        component_set_size_hints(c, gauge_small_on_img.width * size, gauge_small_on_img.height);
    }

    // Set callbacks
    widget_set_obj(c, local);
    widget_set_render_cb(c, gauge_render);
    widget_set_free_cb(c, gauge_free);
    return c;
}
