#include "game/scenes/mechlab/lab_dashboard.h"
#include "game/gui/gauge.h"
#include "game/gui/xysizer.h"
#include "game/gui/label.h"
#include "utils/log.h"

component* lab_dashboard_create(scene *s, dashboard_widgets *dw) {
    component *xy = xysizer_create();

    // Bars and texts
    dw->endurance = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->endurance, 12, 124, -1, -1);

    return xy;
}

void lab_dashboard_update() {
    // Update widgets here from pilot 1 data
}