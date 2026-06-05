#include "game/gui/portrait.h"
#include "formats/error.h"
#include "formats/pic.h"
#include "game/gui/widget.h"
#include "resources/ids.h"
#include "resources/modmanager.h"
#include "resources/resource_files.h"
#include "resources/sprite.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/vga_extended_palette.h"
#include "video/vga_state.h"
#include "video/video.h"

// Local small gauge type
typedef struct portrait {
    sprite *img;
    int max;
    int selected;
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

int portrait_load(sd_sprite *s, vga_palette *pal, int pilot_id) {
    return portrait_load_with_slot(s, pal, pilot_id, 0, NULL);
}

int portrait_load_with_slot(sd_sprite *s, vga_palette *pal, int pilot_id, int slot_index,
                            vga_color portrait_custom_out[64]) {
    log_debug("portrait_load_with_slot: pilot_id=%d slot=%d output=%p", pilot_id, slot_index,
              (void *)portrait_custom_out);
    const path filename = get_resource_filename(get_resource_file(PIC_PLAYERS));

    // Load PIC file and make a surface
    sd_pic_file pics;
    sd_pic_create(&pics);
    int ret = sd_pic_load(&pics, &filename);
    if(ret != SD_SUCCESS) {
        log_error("Could not load PIC file %s: %s", path_c(&filename), sd_get_error(ret));
        return ret;
    } else {
        log_debug("PIC file %s loaded, selecting picture %d.", path_c(&filename), pilot_id);
    }

    modmanager_get_player_pics(&pics);

    sd_sprite_free(s);
    // Create new
    const sd_pic_photo *photo = sd_pic_get(&pics, pilot_id);
    sd_sprite_copy(s, photo->sprite);

    // For mod portraits, overwrite the first 48 expanded common entries
    // with the portrait's own base colors from the PIC file. This way
    // the remap sends 0x00-0x2F to 0x24C+ where the portrait's colors are.
    // For original portraits, the sprite doesn't reference 0x00-0x2F
    // (illegal indices), so the hardcoded expanded common colors are fine.
    palette_copy(pal, &photo->pal, 0, 48);

    // Copy custom portrait colors into extended palette at the selected slot
    for(int c = 0; c < 64; c++) {
        vga_state_set_base_palette_index(VGA_EXT_SLOT1_START + (slot_index * VGA_EXT_SLOT_SIZE) + c,
                                         &photo->portrait_custom[c]);
    }

    // Copy custom colors to output buffer for caller to persist (e.g. in chr->portrait_custom)
    if(portrait_custom_out) {
        memcpy(portrait_custom_out, photo->portrait_custom, 64 * sizeof(vga_color));
        log_debug("portrait_load: copied custom colors to output (custom[0]=%d/%d/%d)", portrait_custom_out[0].r,
                  portrait_custom_out[0].g, portrait_custom_out[0].b);
    }

    // Free pics
    sd_pic_free(&pics);

    return SD_SUCCESS;
}

void portrait_select(component *c, int pilot_id) {
    portrait_select_with_slot(c, pilot_id, 0);
}

void portrait_select_with_slot(component *c, int pilot_id, int slot_index) {
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
    portrait_load_with_slot(&spr, &pal, pilot_id, slot_index, NULL);

    int sprite_remap_type = SPRITE_REMAP_PORTRAIT_1 + slot_index;
    const vga_remap_table *remap = vga_extended_palette_get_sprite_remap(sprite_remap_type);
    sprite_create(local->img, &spr, -1, remap);
    sd_sprite_free(&spr);

    // Position and size hints for the gui component
    // These are set on layout function call
    component_set_size_hints(c, local->img->data->w, local->img->data->h);

    // Save some information
    local->selected = pilot_id;
    local->max = 4; // TODO pics.photo_count;
}

void portrait_next(component *c) {
    portrait *local = widget_get_obj(c);
    int select = local->selected + 1;
    if(select >= local->max) {
        select = 0;
    }
    portrait_select(c, select);
}

void portrait_prev(component *c) {
    portrait *local = widget_get_obj(c);
    int select = local->selected - 1;
    if(select < 0) {
        select = local->max - 1;
    }
    portrait_select(c, select);
}

int portrait_selected(component *c) {
    portrait *local = widget_get_obj(c);
    return local->selected;
}

void portrait_set_from_sprite(component *c, sd_sprite *spr, int slot_index, const vga_color *portrait_custom) {
    portrait *local = widget_get_obj(c);
    // Free old image
    if(local->img != NULL) {
        sprite_free(local->img);
        omf_free(local->img);
    }

    local->img = omf_calloc(1, sizeof(sprite));

    int sprite_remap_type = SPRITE_REMAP_PORTRAIT_1 + slot_index;
    const vga_remap_table *remap = vga_extended_palette_get_sprite_remap(sprite_remap_type);
    sprite_create(local->img, spr, -1, remap);

#ifdef USE_EXTENDED_PALETTE
    // Copy custom portrait colors into extended palette at the selected slot
    if(portrait_custom) {
        for(int i = 0; i < 64; i++) {
            vga_state_set_base_palette_index(VGA_EXT_SLOT1_START + (slot_index * VGA_EXT_SLOT_SIZE) + i,
                                             &portrait_custom[i]);
        }
    }
#endif
    component_set_size_hints(c, local->img->data->w, local->img->data->h);
}

component *portrait_create(int pilot_id) {
    component *c = widget_create();
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    // Local information
    portrait *local = omf_calloc(1, sizeof(portrait));
    local->max = 0;
    local->selected = 0;

    // Set callbacks
    widget_set_obj(c, local);
    widget_set_render_cb(c, portrait_render);
    widget_set_free_cb(c, portrait_free);

    // Update graphics
    portrait_select(c, pilot_id);
    return c;
}
