#include "game/scenes/mechlab/lab_dashboard.h"
#include "game/gui/gauge.h"
#include "utils/log.h"

component* lab_dashboard_create(scene *s) {
    component *c = gauge_create(GAUGE_BIG, 8, 3);
    return c;
}
