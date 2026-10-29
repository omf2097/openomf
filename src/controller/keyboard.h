#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "controller/controller.h"
#include "utils/scancode.h"

#define KEYBOARD_INPUT_BUFFER_SIZE 16

typedef struct keyboard_keys_t keyboard_keys;
typedef struct keyboard_t keyboard;

struct keyboard_keys_t {
    omf_scancode jump_up;
    omf_scancode jump_right;
    omf_scancode walk_right;
    omf_scancode duck_forward;
    omf_scancode duck;
    omf_scancode duck_back;
    omf_scancode walk_back;
    omf_scancode jump_left;
    omf_scancode punch;
    omf_scancode kick;
};

struct keyboard_t {
    keyboard_keys *keys;
};

void keyboard_create(controller *ctrl, keyboard_keys *keys);
void keyboard_free(controller *ctrl);
union SDL_Event;
int keyboard_binds_key(controller *ctrl, union SDL_Event *event);

void keyboard_menu_poll(controller *ctrl, ctrl_event **ev);

#endif // KEYBOARD_H
