#ifndef GAUGE_H
#define GAUGE_H

#include "game/gui/component.h"

typedef enum gauge_type
{
    GAUGE_SMALL,
    GAUGE_BIG
} gauge_type;

component *gauge_create(gauge_type type, int size, int lit);
void gauge_set_lit(component *gauge, int lit);
void gauge_set_size(component *gauge, int size);
int gauge_get_lit(component *gauge);
int gauge_get_size(component *gauge);

#endif // GAUGE_H
