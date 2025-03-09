#include "game/gui/gui_frame.h"
#include "utils/allocator.h"

typedef struct gui_frame {
    int x;
    int y;
    int w;
    int h;
    component *root_node;
    gui_theme theme;
} gui_frame;


gui_frame *gui_frame_create(const gui_theme *theme, int x, int y, int w, int h) {
    gui_frame *frame = omf_calloc(1, sizeof(gui_frame));
    memcpy(&frame->theme, theme, sizeof(gui_theme));
    frame->x = x;
    frame->y = y;
    frame->w = w;
    frame->h = h;
    return frame;
}

void gui_frame_set_root(gui_frame *frame, component *root_node) {
    if(root_node == frame->root_node) {
        return;
    }
    if(frame->root_node != NULL) {
        component_free(frame->root_node);
    }
    frame->root_node = root_node;
    component_set_theme(root_node, &frame->theme);
}

void gui_frame_free(gui_frame *frame) {
    if(frame == NULL) {
        return;
    }
    if(frame->root_node) {
        component_free(frame->root_node);
    }
    omf_free(frame);
}

component *gui_frame_find(gui_frame *frame, int id) {
    if(frame->root_node) {
        return component_find(frame->root_node, id);
    }
    return NULL;
}

component *gui_frame_get_root(const gui_frame *frame) {
    return frame->root_node;
}

void gui_frame_get_measurements(const gui_frame *frame, int *x, int *y, int *w, int *h) {
    if(x != NULL)
        *x = frame->x;
    if(y != NULL)
        *y = frame->y;
    if(w != NULL)
        *w = frame->w;
    if(h != NULL)
        *h = frame->h;
}

void gui_frame_tick(gui_frame *frame) {
    if(frame->root_node) {
        component_tick(frame->root_node);
    }
}

void gui_frame_render(gui_frame *frame) {
    if(frame->root_node) {
        component_render(frame->root_node);
    }
}

int gui_frame_event(gui_frame *frame, SDL_Event *event) {
    if(frame->root_node) {
        return component_event(frame->root_node, event);
    }
    return 1;
}

int gui_frame_action(gui_frame *frame, int action) {
    if(frame->root_node) {
        return component_action(frame->root_node, action);
    }
    return 1;
}

void gui_frame_layout(gui_frame *frame) {
    if(frame->root_node) {
        component_layout(frame->root_node, frame->x, frame->y, frame->w, frame->h);
    }
}
