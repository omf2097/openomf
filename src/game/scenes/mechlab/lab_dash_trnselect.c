#include "game/scenes/mechlab/lab_dash_trnselect.h"
#include "game/gui/label.h"
#include "game/gui/trnselect.h"
#include "game/gui/spriteimage.h"
#include "game/gui/textinput.h"
#include "game/gui/xysizer.h"
#include "resources/bk.h"
#include "resources/languages.h"

component *lab_dash_trnselect_create(scene *s, trnselect_widgets *tw) {
    component *xy = xysizer_create();

    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = color_create(0, 0, 123, 255);

    // Pilot image
    tw->trnselect = trnselect_create();
    xysizer_attach(xy, tw->trnselect, 12, -1, -1, -1);

    return xy;
}
