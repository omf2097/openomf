#ifndef _LAB_NEWPLAYER_H
#define _LAB_NEWPLAYER_H

#include "game/gui/component.h"
#include "game/protos/scene.h"

typedef struct {
    component *input;
} newplayer_widgets;

component* lab_newplayer_create(scene *s, newplayer_widgets *nw);

#endif // _LAB_NEWPLAYER_H
