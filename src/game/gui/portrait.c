#include "game/gui/portrait.h"
#include "formats/error.h"
#include "formats/pic.h"
#include "game/gui/widget.h"
#include "resources/pathmanager.h"
#include "resources/sprite.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/video.h"

// Local small gauge type
typedef struct portrait {
    sprite *img;
    int max;
    int selected;
    int pic_id;
} portrait;

static void portrait_render(component *c) {
    portrait *g = widget_get_obj(c);
    if(g->img != NULL) {
        video_draw(g->img->data, c->x, c->y);
    }
}

static void portrait_free(component *c) {
    portrait *g = widget_get_obj(c);
    sprite_free(g->img);
    omf_free(g->img);
    omf_free(g);
}

int portrait_load(sd_sprite *s, vga_palette *pal, int pic_id, int pilot_id) {
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

    sd_sprite_free(s);
    // Create new
    const sd_pic_photo *photo = sd_pic_get(&pics, pilot_id);
    sd_sprite_copy(s, photo->sprite);
    palette_copy(pal, &photo->pal, 0, 48);
    // Free pics
    sd_pic_free(&pics);

    return SD_SUCCESS;
}

void portrait_select(component *c, int pic_id, int pilot_id) {
    portrait *local = widget_get_obj(c);

    // Free old image
    if(local->img != NULL) {
        sprite_free(local->img);
        omf_free(local->img);
    }

    local->img = omf_calloc(1, sizeof(sprite));
    sd_sprite spr;
    sd_sprite_create(&spr);
    vga_palette pal;
    portrait_load(&spr, &pal, pic_id, pilot_id);

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

void portrait_next(component *c) {
    portrait *local = widget_get_obj(c);
    int select = local->selected + 1;
    if(select >= local->max) {
        select = 0;
    }
    portrait_select(c, local->pic_id, select);
}

void portrait_prev(component *c) {
    portrait *local = widget_get_obj(c);
    int select = local->selected - 1;
    if(select < 0) {
        select = local->max - 1;
    }
    portrait_select(c, local->pic_id, select);
}

int portrait_selected(component *c) {
    portrait *local = widget_get_obj(c);
    return local->selected;
}

void portrait_set_from_sprite(component *c, sd_sprite *spr) {
    portrait *local = widget_get_obj(c);
    // Free old image
    if(local->img != NULL) {
        sprite_free(local->img);
        omf_free(local->img);
    }

    local->img = omf_calloc(1, sizeof(sprite));

    sprite_create(local->img, spr, -1);
    component_set_size_hints(c, local->img->data->w, local->img->data->h);
}

component *portrait_create(int pic_id, int pilot_id) {
    component *c = widget_create();
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    // Local information
    portrait *local = omf_calloc(1, sizeof(portrait));
    local->max = 0;
    local->selected = 0;
    local->pic_id = -1;

    // Set callbacks
    widget_set_obj(c, local);
    widget_set_render_cb(c, portrait_render);
    widget_set_free_cb(c, portrait_free);

    // Update graphics
    portrait_select(c, pic_id, pilot_id);
    return c;
}
