#include "game/gui/filler.h"
#include "game/gui/component.h"

component *filler_create(void) {
    component *c = component_create();
    component_disable(c, true);
    component_set_supported(c, true, false, false);
    return c;
}
