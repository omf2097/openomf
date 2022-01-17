#ifndef LAB_DASH_NEWPLAYER_H
#define LAB_DASH_NEWPLAYER_H

#include "game/gui/component.h"
#include "game/protos/scene.h"

typedef struct {
    component *input;
} newplayer_widgets;

component *lab_dash_newplayer_create(scene *s, newplayer_widgets *nw);

#endif // LAB_DASH_NEWPLAYER_H
