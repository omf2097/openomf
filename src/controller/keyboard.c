#include "controller/keyboard.h"
#include "utils/allocator.h"
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
    if ( state[k->keys->jump_left]) {
        keyboard_cmd(ctrl, ACT_UP|ACT_LEFT, ev);
    } else if ( state[k->keys->duck_back]) {
        keyboard_cmd(ctrl, ACT_DOWN|ACT_LEFT, ev);
    } else if ( state[k->keys->jump_right]) {
        keyboard_cmd(ctrl, ACT_UP|ACT_RIGHT, ev);
    } else if ( state[k->keys->duck_forward]) {
        keyboard_cmd(ctrl, ACT_DOWN|ACT_RIGHT, ev);
    }
    
    if ( state[k->keys->walk_back] && state[k->keys->jump_up]) {
        keyboard_cmd(ctrl, ACT_UP|ACT_LEFT, ev);
    } else if ( state[k->keys->walk_back] && state[k->keys->duck]) {
        keyboard_cmd(ctrl, ACT_DOWN|ACT_LEFT, ev);
    } else if ( state[k->keys->walk_right] && state[k->keys->jump_up]) {
        keyboard_cmd(ctrl, ACT_UP|ACT_RIGHT, ev);
    } else if ( state[k->keys->walk_right] && state[k->keys->duck]) {
        keyboard_cmd(ctrl, ACT_DOWN|ACT_RIGHT, ev);
    } else if ( state[k->keys->walk_right]) {
        keyboard_cmd(ctrl, ACT_RIGHT, ev);
    } else if ( state[k->keys->walk_back]) {
        keyboard_cmd(ctrl, ACT_LEFT, ev);
    } else if ( state[k->keys->jump_up]) {
        keyboard_cmd(ctrl, ACT_UP, ev);
    } else if ( state[k->keys->duck]) {
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
            sc == k->keys->jump_up ||
            sc == k->keys->jump_right ||
            sc == k->keys->walk_right ||
            sc == k->keys->duck_forward ||
            sc == k->keys->duck ||
            sc == k->keys->duck_back ||
            sc == k->keys->walk_back ||
            sc == k->keys->jump_left ||
            sc == k->keys->kick ||
            sc == k->keys->punch ||
            sc == k->keys->escape) {
        return 1;
    }
    return 0;
}

void keyboard_create(controller *ctrl, keyboard_keys *keys, int delay) {
    keyboard *k = omf_calloc(1, sizeof(keyboard));
    k->keys = keys;
    k->last = 0;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->poll_fun = &keyboard_poll;
}
