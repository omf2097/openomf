#include "game/gui/pilotpic.h"
#include "formats/error.h"
#include "formats/pic.h"
#include "game/gui/widget.h"
#include "resources/pathmanager.h"
#include "resources/sprite.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/video.h"

// Local small gauge type
typedef struct {
    sprite *img;
    int max;
    int selected;
    int pic_id;
} pilotpic;

static void pilotpic_render(component *c) {
    pilotpic *g = widget_get_obj(c);
    if(g->img != NULL) {
        video_render_sprite(g->img->data, c->x, c->y, BLEND_ALPHA, 0);
    }
}

static void pilotpic_free(component *c) {
    pilotpic *g = widget_get_obj(c);
    sprite_free(g->img);
    omf_free(g->img);
    omf_free(g);
}

void pilotpic_select(component *c, int pic_id, int pilot_id) {
    pilotpic *local = widget_get_obj(c);

    // Free old image
    if(local->img != NULL) {
        sprite_free(local->img);
        omf_free(local->img);
    }

    // Find pic file handle
    const char *filename = pm_get_resource_path(pic_id);
    if(filename == NULL) {
        PERROR("Could not find requested PIC file handle.");
        return;
    }

    // Load PIC file and make a surface
    sd_pic_file pics;
    sd_pic_create(&pics);
    int ret = sd_pic_load(&pics, filename);
    if(ret != SD_SUCCESS) {
        PERROR("Could not load PIC file %s: %s", filename, sd_get_error(ret));
        return;
    } else {
        DEBUG("PIC file %s loaded, selecting picture %d.", get_resource_name(pic_id), pilot_id);
    }

    // Create new
    const sd_pic_photo *photo = sd_pic_get(&pics, pilot_id);
    local->img = omf_calloc(1, sizeof(sprite));
    sprite_create(local->img, photo->sprite, -1);

    // Position and size hints for the gui component
    // These are set on layout function call
    component_set_size_hints(c, local->img->data->w, local->img->data->h);

    // Save some information
    local->selected = pilot_id;
    local->max = pics.photo_count;
    local->pic_id = pic_id;

    // Free pics
    sd_pic_free(&pics);
}

void pilotpic_next(component *c) {
    pilotpic *local = widget_get_obj(c);
    int select = local->selected + 1;
    if(select >= local->max) {
        select = 0;
    }
    pilotpic_select(c, local->pic_id, select);
}

void pilotpic_prev(component *c) {
    pilotpic *local = widget_get_obj(c);
    int select = local->selected - 1;
    if(select < 0) {
        select = local->max - 1;
    }
    pilotpic_select(c, local->pic_id, select);
}

component *pilotpic_create(int pic_id, int pilot_id) {
    component *c = widget_create();
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    // Local information
    pilotpic *local = omf_calloc(1, sizeof(pilotpic));
    local->max = 0;
    local->selected = 0;
    local->pic_id = -1;

    // Set callbacks
    widget_set_obj(c, local);
    widget_set_render_cb(c, pilotpic_render);
    widget_set_free_cb(c, pilotpic_free);

    // Update graphics
    pilotpic_select(c, pic_id, pilot_id);
    return c;
}
