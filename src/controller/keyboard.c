#include "controller/keyboard.h"

void keyboard_create(controller *ctrl, har *har, keyboard_keys *keys) {
    keyboard *k = malloc(sizeof(keyboard));
    k->keys = keys;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->har = har;
}

void keyboard_free(controller *ctrl) {
    keyboard *k = ctrl->data;
    free(k->keys);
    free(k);
}

int keyboard_handle(controller *ctrl, SDL_Event *event) {
    // If key == sdl_event.key, then har_act(har, ACT_KICK) or something.
    // If key was handled here, return 0. Otherwise 1.
    return 1;
}