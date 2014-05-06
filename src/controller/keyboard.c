#include "controller/keyboard.h"
#include "utils/log.h"
#include <stdlib.h>

void keyboard_free(controller *ctrl) {
    keyboard *k = ctrl->data;
    free(k->keys);
    free(k);
}

void keyboard_cmd(controller *ctrl, int action, ctrl_event **ev) {
    keyboard *k = ctrl->data;
    if (ctrl->repeat && action != ACT_KICK && action != ACT_PUNCH && action != ACT_ESC) {
        controller_cmd(ctrl, action, ev);
    } else if (!(k->last & action)) {
        controller_cmd(ctrl, action, ev);
    }
    k->current |= action;
}

int keyboard_poll(controller *ctrl, ctrl_event **ev) {
    keyboard *k = ctrl->data;
    k->current = 0;
    const unsigned char *state = SDL_GetKeyboardState(NULL);
    if ( state[k->keys->left] && state[k->keys->up]) {
        keyboard_cmd(ctrl, ACT_UPLEFT, ev);
    } else if ( state[k->keys->left] && state[k->keys->down]) {
        keyboard_cmd(ctrl, ACT_DOWNLEFT, ev);
    } else  if ( state[k->keys->right] && state[k->keys->up]) {
        keyboard_cmd(ctrl, ACT_UPRIGHT, ev);
    } else  if ( state[k->keys->right] && state[k->keys->down]) {
        keyboard_cmd(ctrl, ACT_DOWNRIGHT, ev);
    } else if ( state[k->keys->right]) {
        keyboard_cmd(ctrl, ACT_RIGHT, ev);
    } else if ( state[k->keys->left]) {
        keyboard_cmd(ctrl, ACT_LEFT, ev);
    } else if ( state[k->keys->up]) {
        keyboard_cmd(ctrl, ACT_UP, ev);
    } else if ( state[k->keys->down]) {
        keyboard_cmd(ctrl, ACT_DOWN, ev);
    }

    if (state[k->keys->punch]) {
        keyboard_cmd(ctrl, ACT_PUNCH, ev);
    }  else if (state[k->keys->kick]) {
        keyboard_cmd(ctrl, ACT_KICK, ev);
    }

    if (state[k->keys->escape]) {
        keyboard_cmd(ctrl, ACT_ESC, ev);
    }

    if (k->current == 0) {
        keyboard_cmd(ctrl, ACT_STOP, ev);
    }

    k->last = k->current;
    return 0;
}

int keyboard_binds_key(controller *ctrl, SDL_Event *event) {
    keyboard *k = ctrl->data;
    SDL_Scancode sc = event->key.keysym.scancode;
    if (
            sc == k->keys->up ||
            sc == k->keys->down ||
            sc == k->keys->left ||
            sc == k->keys->right ||
            sc == k->keys->kick ||
            sc == k->keys->punch ||
            sc == k->keys->escape) {
        return 1;
    }
    return 0;
}

void keyboard_create(controller *ctrl, keyboard_keys *keys, int delay) {
    keyboard *k = malloc(sizeof(keyboard));
    k->keys = keys;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->poll_fun = &keyboard_poll;
}
