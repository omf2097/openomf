#include <stdio.h>
#include <string.h>

#include "formats/tournament.h"
#include "game/gui/label.h"
#include "game/gui/trnselect.h"
#include "game/gui/widget.h"
#include "resources/sprite.h"
#include "resources/trnmanager.h"
#include "utils/allocator.h"
#include "utils/list.h"
#include "utils/log.h"
#include "video/vga_state.h"
#include "video/video.h"

// Local small gauge type
typedef struct {
    sprite *img;
    list *tournaments;
    component *label;
    int max;
    int selected;
} trnselect;

static void trnselect_render(component *c) {
    trnselect *g = widget_get_obj(c);

    video_draw(g->img->data, c->x + g->img->pos.x, c->y + g->img->pos.y);
    if(g->label) {
        component_render(g->label);
    }
}

static void load_description(component **c, const sd_tournament_locale *locale) {
    log_debug("width %d, center %d, vmove %d size %d color %d", locale->desc_width, locale->desc_center,
              locale->desc_vmove, locale->desc_size, locale->desc_color);

    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.halign = TEXT_CENTER;
    tconf.valign = TEXT_MIDDLE;
    if(locale->desc_color >= 0) {
        tconf.cforeground = locale->desc_color;
    } else {
        // WAR invitational seems to use this color, none is specified
        tconf.cforeground = 0xa5;
    }

    component_free(*c);
    *c = label_create(&tconf, locale->stripped_description);

    int x = 0;
    if(locale->desc_center != 0) {
        x = locale->desc_center - (locale->desc_width / 2);
    } else if(locale->desc_width != 320) {
        x = (320 - locale->desc_width) / 2;
    }

    component_layout(*c, x, locale->desc_vmove, locale->desc_width, 130 - locale->desc_vmove);
}

static void trnselect_free(component *c) {
    trnselect *g = widget_get_obj(c);
    vga_state_pop_palette(); // Recover previous palette
    sprite_free(g->img);
    omf_free(g->img);
    list_free(g->tournaments);
    omf_free(g->tournaments);
    component_free(g->label);
    omf_free(g);
}

void trnselect_next(component *c) {
    trnselect *local = widget_get_obj(c);
    local->selected++;
    if(local->selected >= local->max) {
        local->selected = 0;
    }
    sd_tournament_file *trn = list_get(local->tournaments, local->selected);
    sd_sprite *logo = trn->locales[0]->logo;
    vga_state_set_base_palette_from_range(&trn->pal, 128, 128, 40);
    load_description(&local->label, trn->locales[0]);
    sprite_free(local->img);
    sprite_create(local->img, logo, -1);
}

void trnselect_prev(component *c) {
    trnselect *local = widget_get_obj(c);
    local->selected--;
    if(local->selected < 0) {
        local->selected = local->max - 1;
    }
    sd_tournament_file *trn = list_get(local->tournaments, local->selected);
    sd_sprite *logo = trn->locales[0]->logo;
    vga_state_set_base_palette_from_range(&trn->pal, 128, 128, 40);
    load_description(&local->label, trn->locales[0]);
    sprite_free(local->img);
    sprite_create(local->img, logo, -1);
}

sd_tournament_file *trnselect_selected(component *c) {
    trnselect *local = widget_get_obj(c);
    return list_get(local->tournaments, local->selected);
}

component *trnselect_create(void) {
    component *c = widget_create();
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    // Local information
    trnselect *local = omf_calloc(1, sizeof(trnselect));
    local->selected = 0;
    local->tournaments = trnlist_init();
    local->max = list_size(local->tournaments);
    local->img = omf_calloc(1, sizeof(sprite));

    local->label = NULL;

    vga_state_push_palette(); // Backup the current palette

    sd_tournament_file *trn = list_get(local->tournaments, local->selected);
    sd_sprite *logo = trn->locales[0]->logo;
    vga_state_set_base_palette_from_range(&trn->pal, 128, 128, 40);
    load_description(&local->label, trn->locales[0]);

    sprite_create(local->img, logo, -1);

    // Set callbacks
    widget_set_obj(c, local);
    widget_set_render_cb(c, trnselect_render);
    widget_set_free_cb(c, trnselect_free);

    return c;
}
