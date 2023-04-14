#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "controller/controller.h"
#include <SDL.h>

typedef struct {
    int x_axis;
    int y_axis;
    int dpad[4];
    int punch;
    int kick;
    int escape;
} joystick_keys;

typedef struct {
    SDL_GameController *joy;
    SDL_Haptic *haptic;
    joystick_keys *keys;
    int rumble;
    int last;
    int current;
} joystick;

int joystick_create(controller *ctrl, int joystick_id);
void joystick_free(controller *ctrl);

int joystick_count();
int joystick_nth_id(int n);
int joystick_name_to_id(const char *name, int offset);
int joystick_offset(int id, const char *name);

#endif // JOYSTICK_H
