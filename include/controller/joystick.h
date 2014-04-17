#ifndef _joystick_H
#define _joystick_H

#include "controller/controller.h"
#include <SDL2/SDL.h>

typedef struct joystick_keys_t joystick_keys;
typedef struct joystick_t joystick;

struct joystick_keys_t {
    int x_axis;
    int y_axis;
    int punch;
    int kick;
};

struct joystick_t {
    SDL_GameController *joy;
    joystick_keys *keys;
    int last;
    int current;
};

void joystick_create(controller *ctrl, int joystick_id);
void joystick_free(controller *ctrl);

#endif // _joystick_H
