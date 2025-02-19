#ifndef FRAME_H
#define FRAME_H

#include "game/gui/component.h"
#include "game/gui/theme.h"

typedef struct gui_frame gui_frame;

gui_frame *gui_frame_create(const gui_theme *theme, int x, int y, int w, int h);
void gui_frame_set_root(gui_frame *frame, component *root_node);
component *gui_frame_get_root(const gui_frame *frame);
void gui_frame_free(gui_frame *frame);

component *gui_frame_find(gui_frame *frame, int id);

void gui_frame_get_measurements(const gui_frame *frame, int *x, int *y, int *w, int *h);

void gui_frame_tick(gui_frame *frame);
void gui_frame_render(gui_frame *frame);
int gui_frame_event(gui_frame *frame, SDL_Event *event);
int gui_frame_action(gui_frame *frame, int action);
void gui_frame_layout(gui_frame *frame);

#endif // FRAME_H
