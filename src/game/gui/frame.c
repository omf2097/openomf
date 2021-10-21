#include "game/gui/frame.h"
#include "utils/allocator.h"

guiframe* guiframe_create(int x, int y, int w, int h) {
    guiframe *frame = omf_calloc(1, sizeof(guiframe));
    frame->x = x;
    frame->y = y;
    frame->w = w;
    frame->h = h;
    return frame;
}

void guiframe_set_root(guiframe *frame, component *root_node) {
    if(root_node == frame->root_node) {
        return;
    }
    if(frame->root_node != NULL) {
        component_free(frame->root_node);
    }
    frame->root_node = root_node;
}

void guiframe_free(guiframe *frame) {
    if(frame == NULL) {
        return;
    }
    if(frame->root_node) {
        component_free(frame->root_node);
    }
    omf_free(frame);
}

component* guiframe_find(guiframe *frame, int id) {
    if(frame->root_node) {
        return component_find(frame->root_node, id);
    }
    return NULL;
}

component* guiframe_get_root(const guiframe *frame) {
    return frame->root_node;
}

void guiframe_tick(guiframe *frame) {
    if(frame->root_node) {
        component_tick(frame->root_node);
    }
}

void guiframe_render(guiframe *frame) {
    if(frame->root_node) {
        component_render(frame->root_node);
    }
}

int guiframe_event(guiframe *frame, SDL_Event *event) {
    if(frame->root_node) {
        return component_event(frame->root_node, event);
    }
    return 1;
}

int guiframe_action(guiframe *frame, int action) {
    if(frame->root_node) {
        return component_action(frame->root_node, action);
    }
    return 1;
}

void guiframe_layout(guiframe *frame) {
    if(frame->root_node) {
        component_layout(frame->root_node, frame->x, frame->y, frame->w, frame->h);
    }
}
