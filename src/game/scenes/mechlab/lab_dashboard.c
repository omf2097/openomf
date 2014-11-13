#include "game/scenes/mechlab/lab_dashboard.h"
#include "game/gui/gauge.h"
#include "game/gui/xysizer.h"
#include "game/gui/label.h"
#include "game/gui/pilotpic.h"
#include "resources/ids.h"
#include "utils/log.h"

component* lab_dashboard_create(scene *s, dashboard_widgets *dw) {
    component *xy = xysizer_create();

    text_settings tconf_dark;
    text_defaults(&tconf_dark);
    tconf_dark.font = FONT_SMALL;
    tconf_dark.cforeground = color_create(0, 200, 0, 255);

    // Pilot image
    dw->photo = pilotpic_create(PIC_PLAYERS, 1);
    xysizer_attach(xy, dw->photo, 12, -1, -1, -1);

    // Bars and texts
    xysizer_attach(xy, label_create(&tconf_dark, "Power"), 12, 95, -1, -1);
    dw->power = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->power, 12, 102, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "Agility"), 12, 106, -1, -1);
    dw->agility = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->agility, 12, 113, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "Endurance"), 12, 117, -1, -1);
    dw->endurance = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->endurance, 12, 124, -1, -1);

    return xy;
}

void lab_dashboard_update() {
    // Update widgets here from pilot 1 data
}