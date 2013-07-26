#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <SDL2/SDL.h>

#include "game/har.h"

enum {
    CTRL_TYPE_KEYBOARD,
    CTRL_TYPE_GAMEPAD,
    CTRL_TYPE_NETWORK,
};

typedef struct controller_t controller;

struct controller_t {
    har *har;
    list hooks;
    void (*tick_fun)(controller *ctrl);
    int (*handle_fun)(controller *ctrl, SDL_Event *event);
    void *data;
    int type;
};

void controller_init(controller* ctrl);

void controller_cmd(controller* ctrl, int action);

int controller_event(controller *ctrl, SDL_Event *event);
void controller_tick(controller *ctrl);

void controller_add_hook(controller *ctrl, controller *source, void(*fp)(controller *ctrl, int act_type));

#endif // _CONTROLLER_H
