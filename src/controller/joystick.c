#include "controller/joystick.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdlib.h>

#define UP -32768
#define DOWN 32767
#define LEFT -32768
#define RIGHT 32767
#define CENTER 0

void joystick_free(controller *ctrl) {
    joystick *k = ctrl->data;
    if(k->haptic) {
        SDL_HapticClose(k->haptic);
    }
    SDL_GameControllerClose(k->joy);
    omf_free(k->keys);
    omf_free(k);
}

void joystick_cmd(controller *ctrl, int action, ctrl_event **ev) {
    joystick *k = ctrl->data;
    if(ctrl->repeat && action != ACT_KICK && action != ACT_PUNCH && action != ACT_ESC) {
        controller_cmd(ctrl, action, ev);
    } else if(!(k->last & action)) {
        controller_cmd(ctrl, action, ev);
    }
    k->current |= action;
}

int joystick_count() {
    int valid_joysticks = 0;
    SDL_Joystick *joy;
    for(int i = 0; i < SDL_NumJoysticks(); i++) {
        joy = SDL_JoystickOpen(i);
        if(joy) {
            valid_joysticks++;
        }
        if(SDL_JoystickGetAttached(joy)) {
            SDL_JoystickClose(joy);
        }
    }
    return valid_joysticks;
}

int joystick_nth_id(int n) {
    SDL_Joystick *joy;
    int c = 0;
    for(int i = 0; i < SDL_NumJoysticks(); i++) {
        joy = SDL_JoystickOpen(i);
        if(joy) {
            c++;
            if(SDL_JoystickGetAttached(joy)) {
                SDL_JoystickClose(joy);
            }
            if(c == n) {
                return i;
            }
        }
    }
    return -1;
}

int joystick_offset(int id, const char *name) {
    int offset = 0;
    for(int i = 0; i < id; i++) {
        if(i != id && !strcmp(name, SDL_JoystickNameForIndex(i)))
            offset++;
    }
    return offset;
}

int joystick_name_to_id(const char *name, int offset) {
    for(int i = 0; i < SDL_NumJoysticks(); i++) {
        if(!strcmp(name, SDL_JoystickNameForIndex(i))) {
            if(offset) {
                offset--;
            } else {
                return i;
            }
        }
    }
    return -1;
}

int joystick_poll(controller *ctrl, ctrl_event **ev) {
    joystick *k = ctrl->data;

    k->current = 0;

    Sint16 x_axis = SDL_GameControllerGetAxis(k->joy, k->keys->x_axis);
    Sint16 y_axis = SDL_GameControllerGetAxis(k->joy, k->keys->y_axis);
    int dpadup = SDL_GameControllerGetButton(k->joy, k->keys->dpad[0]);
    int dpaddown = SDL_GameControllerGetButton(k->joy, k->keys->dpad[1]);
    int dpadleft = SDL_GameControllerGetButton(k->joy, k->keys->dpad[2]);
    int dpadright = SDL_GameControllerGetButton(k->joy, k->keys->dpad[3]);

    // joystick input
    // TODO the devide by 2 should be a 'dead zone' variable that can be set in the option menu but this devide works
    // well 99% of the time. Analog Stick (Axis 1) Movement
    if(x_axis <= LEFT / 2 && y_axis <= UP / 2) {
        joystick_cmd(ctrl, ACT_UP | ACT_LEFT, ev);
    } else if(x_axis <= LEFT / 2 && y_axis >= DOWN / 2) {
        joystick_cmd(ctrl, ACT_DOWN | ACT_LEFT, ev);
    } else if(x_axis >= RIGHT / 2 && y_axis <= UP / 2) {
        joystick_cmd(ctrl, ACT_UP | ACT_RIGHT, ev);
    } else if(x_axis >= RIGHT / 2 && y_axis >= DOWN / 2) {
        joystick_cmd(ctrl, ACT_DOWN | ACT_RIGHT, ev);
    } else if(x_axis >= RIGHT / 2) {
        joystick_cmd(ctrl, ACT_RIGHT, ev);
    } else if(x_axis <= LEFT / 2) {
        joystick_cmd(ctrl, ACT_LEFT, ev);
    } else if(y_axis <= UP / 2) {
        joystick_cmd(ctrl, ACT_UP, ev);
    } else if(y_axis >= DOWN / 2) {
        joystick_cmd(ctrl, ACT_DOWN, ev);
    }

    if(dpadup && dpadleft) {
        joystick_cmd(ctrl, ACT_UP | ACT_LEFT, ev);
    } else if(dpaddown && dpadleft) {
        joystick_cmd(ctrl, ACT_DOWN | ACT_LEFT, ev);
    } else if(dpadup && dpadright) {
        joystick_cmd(ctrl, ACT_UP | ACT_RIGHT, ev);
    } else if(dpaddown && dpadright) {
        joystick_cmd(ctrl, ACT_DOWN | ACT_RIGHT, ev);
    } else if(dpadright) {
        joystick_cmd(ctrl, ACT_RIGHT, ev);
    } else if(dpadleft) {
        joystick_cmd(ctrl, ACT_LEFT, ev);
    } else if(dpadup) {
        joystick_cmd(ctrl, ACT_UP, ev);
    } else if(dpaddown) {
        joystick_cmd(ctrl, ACT_DOWN, ev);
    }

    // button input
    if(SDL_GameControllerGetButton(k->joy, k->keys->punch)) {
        joystick_cmd(ctrl, ACT_PUNCH, ev);
    } else if(SDL_GameControllerGetButton(k->joy, k->keys->kick)) {
        joystick_cmd(ctrl, ACT_KICK, ev);
    }

    if(SDL_GameControllerGetButton(k->joy, k->keys->escape)) {
        joystick_cmd(ctrl, ACT_ESC, ev);
    }

    if(k->current == 0) {
        joystick_cmd(ctrl, ACT_STOP, ev);
    }

    k->last = k->current;
    return 0;
}

int joystick_rumble(controller *ctrl, float magnitude, int duration) {
    joystick *k = ctrl->data;
    SDL_HapticRumblePlay(k->haptic, magnitude, duration);
    return 0;
}

int joystick_create(controller *ctrl, int joystick_id) {
    joystick *k = omf_calloc(1, sizeof(joystick));
    k->keys = omf_calloc(1, sizeof(joystick_keys));
    k->keys->x_axis = SDL_CONTROLLER_AXIS_LEFTX;
    k->keys->y_axis = SDL_CONTROLLER_AXIS_LEFTY;
    k->keys->dpad[0] = SDL_CONTROLLER_BUTTON_DPAD_UP;
    k->keys->dpad[1] = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
    k->keys->dpad[2] = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
    k->keys->dpad[3] = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
    k->keys->punch = SDL_CONTROLLER_BUTTON_A;
    k->keys->kick = SDL_CONTROLLER_BUTTON_B;
    k->keys->escape = SDL_CONTROLLER_BUTTON_START;
    k->last = 0;
    k->rumble = 0;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_GAMEPAD;
    ctrl->poll_fun = &joystick_poll;

    k->joy = SDL_GameControllerOpen(joystick_id);
    if(k->joy) {
        k->haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(k->joy));
        if(k->haptic) {
            if(SDL_HapticRumbleSupported(k->haptic)) {
                if(SDL_HapticRumbleInit(k->haptic) == 0) {
                    k->rumble = 1;
                    ctrl->rumble_fun = joystick_rumble;
                } else {
                    DEBUG("Failed to initialize rumble: %s", SDL_GetError());
                }
            } else {
                DEBUG("Rumble not supported");
            }
        } else {
            DEBUG("Haptic not supported");
        }
        return 1;
    }

    DEBUG("failed to open joystick: %s", SDL_GetError());
    return 0;
}
