#include "game/menu/filler.h"
#include "game/menu/widget.h"

component* filler_create() {
    component *c = widget_create();
    component_disable(c, 1);
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;
    return c;
}
