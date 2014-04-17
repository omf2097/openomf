#include "controller/joystick.h"
#include "utils/log.h"
#include <stdlib.h>

#define UP -32768
#define DOWN 32767
#define LEFT -32768
#define RIGHT 32767
#define CENTER 0

void joystick_free(controller *ctrl) {
    joystick *k = ctrl->data;
    SDL_GameControllerClose(k->joy);
    free(k->keys);
    free(k);
}

void joystick_cmd(controller *ctrl, int action, ctrl_event **ev) {
    joystick *k = ctrl->data;
    if (ctrl->repeat || !(k->last & action)) {
        controller_cmd(ctrl, action, ev);
    }
    k->current |= action;
}

int joystick_poll(controller *ctrl, ctrl_event **ev) {
    joystick *k = ctrl->data;

    k->current = 0;

    Sint16 x_axis = SDL_GameControllerGetAxis(k->joy, k->keys->x_axis);
    Sint16 y_axis = SDL_GameControllerGetAxis(k->joy, k->keys->y_axis);

    // joystick input
    // TODO there's no tolerances here, my gamepas has a square hole for the joysticks so it is easy to get the stick into the corner
    if (x_axis == LEFT && y_axis == UP) {
        joystick_cmd(ctrl, ACT_UPLEFT, ev);
    } else if (x_axis == LEFT && y_axis == DOWN) {
        joystick_cmd(ctrl, ACT_DOWNLEFT, ev);
    } else if (x_axis == RIGHT && y_axis == UP) {
        joystick_cmd(ctrl, ACT_UPRIGHT, ev);
    } else if (x_axis == RIGHT && y_axis == DOWN) {
        joystick_cmd(ctrl, ACT_DOWNRIGHT, ev);
    } else if (x_axis == RIGHT) {
        joystick_cmd(ctrl, ACT_RIGHT, ev);
    } else if (x_axis == LEFT) {
        joystick_cmd(ctrl, ACT_LEFT, ev);
    } else if (y_axis == UP) {
        joystick_cmd(ctrl, ACT_UP, ev);
    } else if (y_axis == DOWN) {
        joystick_cmd(ctrl, ACT_DOWN, ev);
    }

    // button input
    if (SDL_GameControllerGetButton(k->joy, k->keys->punch)) {
        joystick_cmd(ctrl, ACT_PUNCH, ev);
    } else if (SDL_GameControllerGetButton(k->joy, k->keys->kick)) {
        joystick_cmd(ctrl, ACT_KICK, ev);
    }

    if (k->current == 0) {
        joystick_cmd(ctrl, ACT_STOP, ev);
    }

    k->last = k->current;
    return 0;
}

int joystick_event(controller *ctrl, SDL_Event *event, ctrl_event **ev) {
    // Do not handle joystick here since it makes the movements unresponsive
    //if (event->type == SDL_KEYUP && (event->key.keysym.sym == k->keys->kick || event->key.keysym.sym == k->keys->punch)) {
    //    return 1;
    //}
    //if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {

    //}
    joystick_poll(ctrl, ev);
    return 0;
}

void joystick_create(controller *ctrl, int joystick_id) {
    joystick *k = malloc(sizeof(joystick));
    k->keys = malloc(sizeof(joystick_keys));
    k->keys->x_axis = SDL_CONTROLLER_AXIS_LEFTX;
    k->keys->y_axis = SDL_CONTROLLER_AXIS_LEFTY;
    k->keys->kick = SDL_CONTROLLER_BUTTON_A;
    k->keys->punch = SDL_CONTROLLER_BUTTON_B;
    k->last = 0;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_GAMEPAD;
    ctrl->poll_fun = &joystick_poll;
    ctrl->event_fun = &joystick_event;

    k->joy = SDL_GameControllerOpen(joystick_id);

}
