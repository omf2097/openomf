#ifndef _GAUGE_H
#define _GAUGE_H

#include "game/gui/component.h"

typedef enum {
    GAUGE_SMALL,
    GAUGE_BIG
} gauge_type;

component* gauge_create(gauge_type type, int size, int lit);
void gauge_set_lit(component *gauge, int lit);
int gauge_get_lit(component *gauge);
int gauge_get_size(component *gauge);

#endif // _GAUGE_H
