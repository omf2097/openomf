#include "game/gui/trnselect.h"
#include "formats/error.h"
#include "formats/tournament.h"
#include "game/gui/widget.h"
#include "resources/pathmanager.h"
#include "resources/sprite.h"
#include "resources/trnmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/list.h"
#include "video/video.h"

// Local small gauge type
typedef struct {
    sprite *img;
    list *tournaments;
    int max;
    int selected;
} trnselect;

static void trnselect_render(component *c) {
    trnselect *g = widget_get_obj(c);

    video_render_sprite(g->img->data, c->x, c->y, BLEND_ALPHA, 0);
}

static void trnselect_free(component *c) {
    trnselect *g = widget_get_obj(c);
    sprite_free(g->img);
    omf_free(g->img);
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
    sprite_create(local->img, logo, -1);
}

sd_tournament_file* trnselect_selected(component *c) {
    trnselect *local = widget_get_obj(c);
    return list_get(local->tournaments, local->selected);
}

component *trnselect_create() {
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

    sd_tournament_file *trn = list_get(local->tournaments, local->selected);
    sd_sprite *logo = trn->locales[0]->logo;
    sprite_create(local->img, logo, -1);


    // Set callbacks
    widget_set_obj(c, local);
    widget_set_render_cb(c, trnselect_render);
    widget_set_free_cb(c, trnselect_free);

    return c;
}
