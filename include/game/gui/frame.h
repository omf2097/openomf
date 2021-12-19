#ifndef FRAME_H
#define FRAME_H

#include "game/gui/component.h"

typedef struct {
    int x;
    int y;
    int w;
    int h;
    component *root_node;
} guiframe;

guiframe* guiframe_create(int x, int y, int w, int h);
void guiframe_set_root(guiframe *frame, component *root_node);
component* guiframe_get_root(const guiframe *frame);
void guiframe_free(guiframe *frame);

component* guiframe_find(guiframe *frame, int id);

void guiframe_tick(guiframe *frame);
void guiframe_render(guiframe *frame);
int guiframe_event(guiframe *frame, SDL_Event *event);
int guiframe_action(guiframe *frame, int action);
void guiframe_layout(guiframe *frame);

#endif // FRAME_H
