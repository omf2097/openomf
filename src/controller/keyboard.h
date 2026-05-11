#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "controller/controller.h"
#include <SDL.h>

#define KEYBOARD_INPUT_BUFFER_SIZE 16

typedef struct keyboard_keys_t keyboard_keys;
typedef struct keyboard_t keyboard;

struct keyboard_keys_t {
    SDL_Scancode jump_up;
    SDL_Scancode jump_right;
    SDL_Scancode walk_right;
    SDL_Scancode duck_forward;
    SDL_Scancode duck;
    SDL_Scancode duck_back;
    SDL_Scancode walk_back;
    SDL_Scancode jump_left;
    SDL_Scancode punch;
    SDL_Scancode kick;
};

struct keyboard_t {
    keyboard_keys *keys;
};

void keyboard_create(controller *ctrl, keyboard_keys *keys);
void keyboard_free(controller *ctrl);
int keyboard_binds_key(controller *ctrl, SDL_Event *event);

void keyboard_menu_poll(controller *ctrl, ctrl_event **ev);

#endif // KEYBOARD_H
