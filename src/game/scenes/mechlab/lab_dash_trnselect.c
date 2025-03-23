#include "game/scenes/mechlab/lab_dash_trnselect.h"
#include "game/gui/label.h"
#include "game/gui/spriteimage.h"
#include "game/gui/textinput.h"
#include "game/gui/trnselect.h"
#include "game/gui/xysizer.h"
#include "resources/bk.h"
#include "resources/languages.h"

bool lab_dash_trnselect_select(component *c, void *userdata) {
    return true;
}

bool lab_dash_trnselect_left(component *c, void *userdata) {
    trnselect_widgets *tw = userdata;
    trnselect_prev(tw->trnselect);
    return true;
}

bool lab_dash_trnselect_right(component *c, void *userdata) {
    trnselect_widgets *tw = userdata;
    trnselect_next(tw->trnselect);
    return true;
}

sd_tournament_file *lab_dash_trnselect_selected(trnselect_widgets *tw) {
    return trnselect_selected(tw->trnselect);
}

component *lab_dash_trnselect_create(scene *s, trnselect_widgets *tw) {
    component *xy = xysizer_create();

    // Pilot image
    tw->trnselect = trnselect_create();
    xysizer_attach(xy, tw->trnselect, -1, -1, -1, -1);

    return xy;
}
