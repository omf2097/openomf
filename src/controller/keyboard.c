#include "controller/keyboard.h"
#include "utils/log.h"
#include <stdlib.h>

void keyboard_free(controller *ctrl) {
    keyboard *k = ctrl->data;
    free(k->keys);
    free(k);
}

int keyboard_poll(controller *ctrl, ctrl_event **ev) {
    keyboard *k = ctrl->data;
    const unsigned char *state = SDL_GetKeyboardState(NULL);
    int handled = 0;

    if ( state[k->keys->left] && state[k->keys->up]) {
        controller_cmd(ctrl, ACT_UPLEFT, ev);
        handled = 1;
    } else if ( state[k->keys->left] && state[k->keys->down]) {
        controller_cmd(ctrl, ACT_DOWNLEFT, ev);
        handled = 1;
    } else  if ( state[k->keys->right] && state[k->keys->up]) {
        controller_cmd(ctrl, ACT_UPRIGHT, ev);
        handled = 1;
    } else  if ( state[k->keys->right] && state[k->keys->down]) {
        controller_cmd(ctrl, ACT_DOWNRIGHT, ev);
        handled = 1;
    } else if ( state[k->keys->right]) {
        controller_cmd(ctrl, ACT_RIGHT, ev);
        handled = 1;
    } else if ( state[k->keys->left]) {
        controller_cmd(ctrl, ACT_LEFT, ev);
        handled = 1;
    } else if ( state[k->keys->up]) {
        controller_cmd(ctrl, ACT_UP, ev);
        handled = 1;
    } else if ( state[k->keys->down]) {
        controller_cmd(ctrl, ACT_DOWN, ev);
        handled = 1;
    }

    if (state[k->keys->punch]) {
        controller_cmd(ctrl, ACT_PUNCH, ev);
        handled = 1;
    } else if (state[k->keys->kick]) {
        controller_cmd(ctrl, ACT_KICK, ev);
        handled = 1;
    }

    // If key was handled here, return 0. Otherwise 1.
    if(!handled) {
        controller_cmd(ctrl, ACT_STOP, ev);
    }
    return 0;
}

int keyboard_event(controller *ctrl, SDL_Event *event, ctrl_event **ev) {
    keyboard *k = ctrl->data;
    SDL_Scancode sc = event->key.keysym.scancode;

    if(event->type == SDL_KEYDOWN) {
        if ( sc == k->keys->left && sc == k->keys->up) {
            controller_cmd(ctrl, ACT_UPLEFT, ev);
        } else if ( sc == k->keys->left && sc == k->keys->down) {
            controller_cmd(ctrl, ACT_DOWNLEFT, ev);
        } else  if ( sc == k->keys->right && sc == k->keys->up) {
            controller_cmd(ctrl, ACT_UPRIGHT, ev);
        } else  if ( sc == k->keys->right && sc == k->keys->down) {
            controller_cmd(ctrl, ACT_DOWNRIGHT, ev);
        } else if ( sc == k->keys->right) {
            controller_cmd(ctrl, ACT_RIGHT, ev);
        } else if ( sc == k->keys->left) {
            controller_cmd(ctrl, ACT_LEFT, ev);
        } else if ( sc == k->keys->up) {
            controller_cmd(ctrl, ACT_UP, ev);
        } else if ( sc == k->keys->down) {
            controller_cmd(ctrl, ACT_DOWN, ev);
        }
        if (sc == k->keys->punch) {
            controller_cmd(ctrl, ACT_PUNCH, ev);
        } else if (sc == k->keys->kick) {
            controller_cmd(ctrl, ACT_KICK, ev);
        }
        return 0;
    }
    return 1;
}

void keyboard_create(controller *ctrl, keyboard_keys *keys) {
    keyboard *k = malloc(sizeof(keyboard));
    k->keys = keys;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->poll_fun = &keyboard_poll;
    ctrl->event_fun = &keyboard_event;
    /*controller_add_hook(ctrl, &hook);*/
}
