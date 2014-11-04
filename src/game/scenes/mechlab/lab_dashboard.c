#include "game/scenes/mechlab/lab_dashboard.h"
#include "game/gui/gauge.h"
#include "game/gui/xysizer.h"
#include "utils/log.h"

component* lab_dashboard_create(scene *s) {
    component *xy = xysizer_create();

    component *c = gauge_create(GAUGE_BIG, 8, 3);
    xysizer_attach(xy, c, 20, 20, -1, -1);

    return xy;
}
