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
    SDL_JoystickClose(k->joy);
    /*free(k->keys);*/
    free(k);
}

int joystick_tick(controller *ctrl, ctrl_event **ev) {
    joystick *k = ctrl->data;

    /*
    // debug output, to help figure out what buttons and such are needed

    int num = SDL_JoystickNumButtons(k->joy);

    for (int i = 0; i < num; i++) {
        if (SDL_JoystickGetButton(k->joy, i)) {
            DEBUG("button %d is down");
        }
    }

    num = SDL_JoystickNumAxes(k->joy);
    for (int i = 0; i < num; i++) {
        DEBUG("axis %d is %d", i, SDL_JoystickGetAxis(k->joy, i));
    }


    num = SDL_JoystickNumHats(k->joy);
    for (int i = 0; i < num; i++) {
        switch(SDL_JoystickGetHat(k->joy, i)) {
            case SDL_HAT_LEFTUP:
                DEBUG("Hat is LEFTUP");
                break;
            case SDL_HAT_LEFTDOWN:
                DEBUG("Hat is LEFTDOWN");
                break;
           case SDL_HAT_RIGHTUP:
                DEBUG("Hat is RIGHTUP");
                break;
           case SDL_HAT_RIGHTDOWN:
                DEBUG("Hat is RIGHTDOWN");
                break;
           case SDL_HAT_RIGHT:
                DEBUG("Hat is RIGHT");
                break;
           case SDL_HAT_LEFT:
                DEBUG("Hat is LEFT");
                break;
           case SDL_HAT_UP:
                DEBUG("Hat is UP");
                break;
           case SDL_HAT_DOWN:
                DEBUG("Hat is DOWN");
                break;
           case SDL_HAT_CENTERED:
                DEBUG("Hat is CENTERED");
                break;
        }
    }

    */

    /*
    // input from a directional pad
    switch(SDL_JoystickGetHat(k->joy, 0)) {
        case SDL_HAT_LEFTUP:
            controller_cmd(ctrl, ACT_UPLEFT, ev);
            break;
        case SDL_HAT_LEFTDOWN:
            controller_cmd(ctrl, ACT_DOWNLEFT, ev);
            break;
        case SDL_HAT_RIGHTUP:
            controller_cmd(ctrl, ACT_UPRIGHT, ev);
            break;
        case SDL_HAT_RIGHTDOWN:
            controller_cmd(ctrl, ACT_DOWNRIGHT, ev);
            break;
        case SDL_HAT_RIGHT:
            controller_cmd(ctrl, ACT_RIGHT, ev);
            break;
        case SDL_HAT_LEFT:
            controller_cmd(ctrl, ACT_LEFT, ev);
            break;
        case SDL_HAT_UP:
            controller_cmd(ctrl, ACT_UP, ev);
            break;
        case SDL_HAT_DOWN:
            controller_cmd(ctrl, ACT_DOWN, ev);
            break;
        case SDL_HAT_CENTERED:
            controller_cmd(ctrl, ACT_STOP, ev);
            break;
    }
    */

    Sint16 x_axis = SDL_JoystickGetAxis(k->joy, 0);
    Sint16 y_axis = SDL_JoystickGetAxis(k->joy, 1);

    // joystick input
    // TODO there's no tolerances here, my gamepas has a square hole for the joysticks so it is easy to get the stick into the corner
    if (x_axis == LEFT && y_axis == UP) {
        controller_cmd(ctrl, ACT_UPLEFT, ev);
    } else if (x_axis == LEFT && y_axis == DOWN) {
        controller_cmd(ctrl, ACT_DOWNLEFT, ev);
    } else if (x_axis == RIGHT && y_axis == UP) {
        controller_cmd(ctrl, ACT_UPRIGHT, ev);
    } else if (x_axis == RIGHT && y_axis == DOWN) {
        controller_cmd(ctrl, ACT_DOWNRIGHT, ev);
    } else if (x_axis == RIGHT) {
        controller_cmd(ctrl, ACT_RIGHT, ev);
    } else if (x_axis == LEFT) {
        controller_cmd(ctrl, ACT_LEFT, ev);
    } else if (y_axis == UP) {
        controller_cmd(ctrl, ACT_UP, ev);
    } else if (y_axis == DOWN) {
        controller_cmd(ctrl, ACT_DOWN, ev);
    } else {
        controller_cmd(ctrl, ACT_STOP, ev);
    }

    // button input
    if (SDL_JoystickGetButton(k->joy, 2)) {
        controller_cmd(ctrl, ACT_PUNCH, ev);
    } else if (SDL_JoystickGetButton(k->joy, 1)) {
        controller_cmd(ctrl, ACT_KICK, ev);
    }

    return 0;
}

int joystick_handle(controller *ctrl, SDL_Event *event, ctrl_event **ev) {
    // Do not handle joystick here since it makes the movements unresponsive
    //if (event->type == SDL_KEYUP && (event->key.keysym.sym == k->keys->kick || event->key.keysym.sym == k->keys->punch)) {
    //    return 1;
    //}
    //if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {

    //}
    joystick_tick(ctrl, ev);
    return 0;
}

void joystick_create(controller *ctrl, int joystick_id) {
    joystick *k = malloc(sizeof(joystick));
    /*k->keys = keys;*/
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_GAMEPAD;
    ctrl->tick_fun = &joystick_tick;
    ctrl->handle_fun = &joystick_handle;

    k->joy = SDL_JoystickOpen(joystick_id);

    if(k->joy) {
        DEBUG("Opened Joystick %d", joystick_id);
        DEBUG(" * Name:              %s", SDL_JoystickNameForIndex(joystick_id));
        DEBUG(" * Number of Axes:    %d", SDL_JoystickNumAxes(k->joy));
        DEBUG(" * Number of Buttons: %d", SDL_JoystickNumButtons(k->joy));
        DEBUG(" * Number of Balls:   %d", SDL_JoystickNumBalls(k->joy));
        DEBUG(" * Number of Hats:    %d", SDL_JoystickNumHats(k->joy));
    }

    /*controller_add_hook(ctrl, &hook);*/
}
