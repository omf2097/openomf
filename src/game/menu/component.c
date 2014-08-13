#include <stdlib.h>

#include "game/menu/component.h"
#include "utils/log.h"

void component_create(component *c) {
    c->x = 0;
    c->y = 0;
    c->w = 0;
    c->h = 0;
    c->selected = 0;
    c->disabled = 0;
    c->obj = NULL;
    c->userdata = NULL;

    // Function pointers
    c->render = NULL;
    c->event = NULL;
    c->layout = component_layout;
    c->tick = NULL;

    // Event functions
    c->click = NULL;
    c->toggle = NULL;
    c->slide = NULL;
}

void component_free(component *c) {

}

void component_layout(component *c, int x, int y, int w, int h) {
    c->x = x;
    c->y = y;
    c->w = w;
    c->h = h;
}

void component_click(component *c){
    if(c->click) {
        c->click(c, c->userdata);
    }
}
