#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "controller/controller.h"
#include <SDL.h>

#define KEYBOARD_INPUT_BUFFER_SIZE 16

typedef struct keyboard_keys_t keyboard_keys;
typedef struct keyboard_t keyboard;

struct keyboard_keys_t {
    unsigned jump_up;
    unsigned jump_right;
    unsigned walk_right;
    unsigned duck_forward;
    unsigned duck;
    unsigned duck_back;
    unsigned walk_back;
    unsigned jump_left;
    unsigned punch;
    unsigned kick;
    unsigned escape;
};

struct keyboard_t {
    keyboard_keys *keys;
    int last;
    int current;
};

void keyboard_create(controller *ctrl, keyboard_keys *keys, int delay);
void keyboard_free(controller *ctrl);
int keyboard_binds_key(controller *ctrl, SDL_Event *event);

#endif // KEYBOARD_H
