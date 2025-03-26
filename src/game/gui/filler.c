#include "game/gui/filler.h"
#include "game/gui/widget.h"

component *filler_create(void) {
    component *c = widget_create();
    component_disable(c, true);
    component_set_supports(c, true, false, false);
    return c;
}
