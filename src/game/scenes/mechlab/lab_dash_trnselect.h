#ifndef LAB_DASH_TRNSELECT_H
#define LAB_DASH_TRNSELECT_H

#include "game/gui/component.h"
#include "game/protos/scene.h"

typedef struct {
    component *input;
} trnselect_widgets;

component *lab_dash_trnselect_create(scene *s, trnselect_widgets *tw);

#endif // LAB_DASH_TRNSELECT_H
