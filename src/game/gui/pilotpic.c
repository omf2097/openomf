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
        video_draw(g->img->data, c->x, c->y);
    }
}

static void pilotpic_free(component *c) {
    pilotpic *g = widget_get_obj(c);
    sprite_free(g->img);
    omf_free(g->img);
    omf_free(g);
}

int pilotpic_load(sd_sprite *sprite, vga_palette *pal, int pic_id, int pilot_id) {
    const char *filename = pm_get_resource_path(pic_id);
    if(filename == NULL) {
        log_error("Could not find requested PIC file handle.");
        return SD_FILE_OPEN_ERROR;
    }

    // Load PIC file and make a surface
    sd_pic_file pics;
    sd_pic_create(&pics);
    int ret = sd_pic_load(&pics, filename);
    if(ret != SD_SUCCESS) {
        log_error("Could not load PIC file %s: %s", filename, sd_get_error(ret));
        return ret;
    } else {
        log_debug("PIC file %s loaded, selecting picture %d.", get_resource_name(pic_id), pilot_id);
    }

    sd_sprite_free(sprite);
    // Create new
    const sd_pic_photo *photo = sd_pic_get(&pics, pilot_id);
    sd_sprite_copy(sprite, photo->sprite);
    palette_copy(pal, &photo->pal, 0, 48);
    // Free pics
    sd_pic_free(&pics);

    return SD_SUCCESS;
}

void pilotpic_select(component *c, int pic_id, int pilot_id) {
    pilotpic *local = widget_get_obj(c);

    // Free old image
    if(local->img != NULL) {
        sprite_free(local->img);
        omf_free(local->img);
    }

    local->img = omf_calloc(1, sizeof(sprite));
    sd_sprite spr;
    sd_sprite_create(&spr);
    vga_palette pal;
    pilotpic_load(&spr, &pal, pic_id, pilot_id);

    sprite_create(local->img, &spr, -1);
    sd_sprite_free(&spr);

    // Position and size hints for the gui component
    // These are set on layout function call
    component_set_size_hints(c, local->img->data->w, local->img->data->h);

    // Save some information
    local->selected = pilot_id;
    local->max = 4; // TODO pics.photo_count;
    local->pic_id = pic_id;
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

int pilotpic_selected(component *c) {
    pilotpic *local = widget_get_obj(c);
    return local->selected;
}

void pilotpic_set_photo(component *c, sd_sprite *spr) {
    pilotpic *local = widget_get_obj(c);
    // Free old image
    if(local->img != NULL) {
        sprite_free(local->img);
        omf_free(local->img);
    }

    local->img = omf_calloc(1, sizeof(sprite));

    sprite_create(local->img, spr, -1);
    component_set_size_hints(c, local->img->data->w, local->img->data->h);
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
