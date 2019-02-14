#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "controller/controller.h"
#include <SDL.h>

#define KEYBOARD_INPUT_BUFFER_SIZE 16

typedef struct keyboard_keys_t keyboard_keys;
typedef struct keyboard_t keyboard;

struct keyboard_keys_t {
    int jump_up;
    int jump_right;
    int walk_right;
    int duck_forward;
    int duck;
    int duck_back;
    int walk_back;
    int jump_left;
    int punch;
    int kick;
    int escape;
};

struct keyboard_t {
    keyboard_keys *keys;
    int last;
    int current;
};

void keyboard_create(controller *ctrl, keyboard_keys *keys, int delay);
void keyboard_free(controller *ctrl);
int keyboard_binds_key(controller *ctrl, SDL_Event *event);

#endif // _KEYBOARD_H
