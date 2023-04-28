#ifndef LAB_DASH_TRNSELECT_H
#define LAB_DASH_TRNSELECT_H

#include "game/gui/component.h"
#include "game/protos/scene.h"

typedef struct {
    component *trnselect;
} trnselect_widgets;

void lab_dash_trnselect_select(component *c, void *userdata);

void lab_dash_trnselect_left(component *c, void *userdata);

void lab_dash_trnselect_right(component *c, void *userdata);

sd_tournament_file *lab_dash_trnselect_selected(trnselect_widgets *tw);

component *lab_dash_trnselect_create(scene *s, trnselect_widgets *tw);

#endif // LAB_DASH_TRNSELECT_H
