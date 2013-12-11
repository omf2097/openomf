#include "controller/keyboard.h"
#include "utils/log.h"
#include <stdlib.h>

void keyboard_free(controller *ctrl) {
    keyboard *k = ctrl->data;
    free(k->keys);
    free(k);
}

int keyboard_tick(controller *ctrl, ctrl_event **ev) {
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

int keyboard_handle(controller *ctrl, SDL_Event *event, ctrl_event **ev) {
    // Do not handle keyboard here since it makes the movements unresponsive
    //if (event->type == SDL_KEYUP && (event->key.keysym.sym == k->keys->kick || event->key.keysym.sym == k->keys->punch)) {
    //    return 1;
    //}
    //if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {

    //}
    keyboard_tick(ctrl, ev);
    return 0;
}

void keyboard_create(controller *ctrl, keyboard_keys *keys) {
    keyboard *k = malloc(sizeof(keyboard));
    k->keys = keys;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->tick_fun = &keyboard_tick;
    ctrl->handle_fun = &keyboard_handle;
    /*controller_add_hook(ctrl, &hook);*/
}
