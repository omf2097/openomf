#include "game/gui/gauge.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include "video/surface.h"
#include "video/video.h"

/* GIMP RGBA C-Source image dump (gauge_small_off.c) */
static const struct {
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[3 * 3 * 4 + 1];
} gauge_small_off_img = {
    3,
    3,
    4,
    "\0\207\0\377\0\\\0\377\0"
    "1\0\377\0\\\0\377\0,\0\377\0\10\0\377\0-\0\377"
    "\0"
    "6\0\377\0\14\0\377",
};

/* GIMP RGBA C-Source image dump (gauge_small_on.c) */
static const struct {
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[3 * 3 * 4 + 1];
} gauge_small_on_img = {
    3,
    3,
    4,
    "\0\377\0\377\0\370\0\377\0\314\0\377\0\317\0\377\0\241\0\377\0y\0\377\0y"
    "\0\377\0N\0\377\0#\0\377",
};

/* GIMP RGBA C-Source image dump (gauge_big_on.c) */
static const struct {
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[8 * 3 * 4 + 1];
} gauge_big_on_img = {
    8,
    3,
    4,
    "\0\377\0\377\0\364\0\377\0\366\0\377\0\366\0\377\0\366\0\377\0\366\0\377"
    "\0\373\0\377\0\314\0\377\0\317\0\377\0\235\0\377\0\241\0\377\0\241\0\377"
    "\0\241\0\377\0\241\0\377\0\246\0\377\0y\0\377\0y\0\377\0J\0\377\0N\0\377"
    "\0N\0\377\0N\0\377\0N\0\377\0S\0\377\0#\0\377",
};

/* GIMP RGBA C-Source image dump (gauge_big_off.c) */
static const struct {
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[8 * 3 * 4 + 1];
} gauge_big_off_img = {
    8,
    3,
    4,
    "\0\207\0\377\0X\0\377\0\\\0\377\0\\\0\377\0\\\0\377\0\\\0\377\0a\0\377\0"
    ""
    "1\0\377\0\\\0\377\0(\0\377\0-\0\377\0-\0\377\0-\0\377\0-\0\377\0"
    "1\0\377"
    "\0\10\0\377\0-\0\377\0"
    "2\0\377\0"
    "2\0\377\0"
    "2\0\377\0"
    "2\0\377\0"
    "2\0\377"
    "\0"
    "6\0\377\0\14\0\377",
};

// Local small gauge type
typedef struct {
    int size;
    int lit;
    gauge_type type;
    surface *img;
} gauge;

static void gauge_update(component *c) {
    gauge *g = widget_get_obj(c);

    // Create ON & OFF surface from gimp C exports
    surface on;
    surface off;
    int move;
    if(g->type == GAUGE_SMALL) {
        move = 3;
        surface_create_from_data(&on, SURFACE_TYPE_RGBA, 3, 3, (const char *)gauge_small_on_img.pixel_data);
        surface_create_from_data(&off, SURFACE_TYPE_RGBA, 3, 3, (const char *)gauge_small_off_img.pixel_data);
    } else {
        move = 8;
        surface_create_from_data(&on, SURFACE_TYPE_RGBA, 8, 3, (const char *)gauge_big_on_img.pixel_data);
        surface_create_from_data(&off, SURFACE_TYPE_RGBA, 8, 3, (const char *)gauge_big_off_img.pixel_data);
    }

    surface_clear(g->img);
    // Blit required stuff to cached surface
    int k = 0;
    int x = 0;
    for(; k < g->lit; k++) {
        surface_rgba_blit(g->img, &on, x, 0);
        x += move;
    }
    for(; k < g->size; k++) {
        surface_rgba_blit(g->img, &off, x, 0);
        x += move;
    }

    // Free up unnecessary surfaces
    surface_free(&on);
    surface_free(&off);

    // Force cached texture update!
    surface_force_refresh(g->img);
}

static void gauge_render(component *c) {
    gauge *g = widget_get_obj(c);
    video_render_sprite(g->img, c->x, c->y, BLEND_ALPHA, 0);
}

static void gauge_free(component *c) {
    gauge *g = widget_get_obj(c);
    surface_free(g->img);
    omf_free(g->img);
    omf_free(g);
}

void gauge_set_lit(component *c, int lit) {
    gauge *g = widget_get_obj(c);
    if(lit != g->lit) {
        g->lit = lit;
        gauge_update(c);
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
        if (g->lit > g->size) {
            g->lit = g->size;
        }
        gauge_update(c);
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

    // Initialize image
    local->img = omf_calloc(1, sizeof(surface));
    int pix = ((type == GAUGE_BIG) ? 8 : 3) * size;
    surface_create(local->img, SURFACE_TYPE_RGBA, pix, 3);
    component_set_size_hints(c, pix, 3);

    // Set callbacks
    widget_set_obj(c, local);
    widget_set_render_cb(c, gauge_render);
    widget_set_free_cb(c, gauge_free);

    // Update graphics
    gauge_update(c);
    return c;
}
