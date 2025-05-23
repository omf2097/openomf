#include "controller/keyboard.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdlib.h>

void keyboard_free(controller *ctrl) {
    keyboard *k = ctrl->data;
    omf_free(k->keys);
    omf_free(k);
}

static inline void keyboard_cmd(controller *ctrl, int action, ctrl_event **ev) {
    controller_cmd(ctrl, action, ev);
}

int keyboard_poll(controller *ctrl, ctrl_event **ev) {
    keyboard *k = ctrl->data;
    ctrl->current = 0;
    const unsigned char *state = SDL_GetKeyboardState(NULL);
    int action = 0;
    if(state[k->keys->jump_left]) {
        action = ACT_UP | ACT_LEFT;
    } else if(state[k->keys->duck_back]) {
        action = ACT_DOWN | ACT_LEFT;
    } else if(state[k->keys->jump_right]) {
        action = ACT_UP | ACT_RIGHT;
    } else if(state[k->keys->duck_forward]) {
        action = ACT_DOWN | ACT_RIGHT;
    } else if(state[k->keys->walk_back] && state[k->keys->jump_up]) {
        action = ACT_UP | ACT_LEFT;
    } else if(state[k->keys->walk_back] && state[k->keys->duck]) {
        action = ACT_DOWN | ACT_LEFT;
    } else if(state[k->keys->walk_right] && state[k->keys->jump_up]) {
        action = ACT_UP | ACT_RIGHT;
    } else if(state[k->keys->walk_right] && state[k->keys->duck]) {
        action = ACT_DOWN | ACT_RIGHT;
    } else if(state[k->keys->walk_right]) {
        action = ACT_RIGHT;
    } else if(state[k->keys->walk_back]) {
        action = ACT_LEFT;
    } else if(state[k->keys->jump_up]) {
        action = ACT_UP;
    } else if(state[k->keys->duck]) {
        action = ACT_DOWN;
    }

    if(state[k->keys->punch]) {
        action |= ACT_PUNCH;
    }

    if(state[k->keys->kick]) {
        action |= ACT_KICK;
    }

    if(action == 0) {
        keyboard_cmd(ctrl, ACT_STOP, ev);
    } else {
        keyboard_cmd(ctrl, action, ev);
    }

    ctrl->last = ctrl->current;
    return 0;
}

int keyboard_binds_key(controller *ctrl, SDL_Event *event) {
    keyboard *k = ctrl->data;
    SDL_Scancode sc = event->key.keysym.scancode;
    if(sc == k->keys->jump_up || sc == k->keys->jump_right || sc == k->keys->walk_right ||
       sc == k->keys->duck_forward || sc == k->keys->duck || sc == k->keys->duck_back || sc == k->keys->walk_back ||
       sc == k->keys->jump_left || sc == k->keys->kick || sc == k->keys->punch) {
        return 1;
    }
    return 0;
}

void keyboard_create(controller *ctrl, keyboard_keys *keys) {
    keyboard *k = omf_calloc(1, sizeof(keyboard));
    k->keys = keys;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->poll_fun = &keyboard_poll;
    ctrl->free_fun = &keyboard_free;
    ctrl->supports_delay = true;
}

void keyboard_menu_poll(controller *ctrl, ctrl_event **ev) {
    const unsigned char *state = SDL_GetKeyboardState(NULL);

    if(ctrl->queued != ACT_NONE) {
        controller_cmd(ctrl, ctrl->queued, ev);
        ctrl->queued = ACT_NONE;
    }

    if(state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_KP_6]) {
        controller_cmd(ctrl, ACT_RIGHT, ev);
    }
    if(state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_KP_4]) {
        controller_cmd(ctrl, ACT_LEFT, ev);
    }
    if(state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_KP_8]) {
        controller_cmd(ctrl, ACT_UP, ev);
    }
    if(state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_KP_2]) {
        controller_cmd(ctrl, ACT_DOWN, ev);
    }
    if(state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_KP_ENTER]) {
        controller_cmd(ctrl, ACT_PUNCH, ev);
    }
    if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_KP_0]) {
        controller_cmd(ctrl, ACT_KICK, ev);
    }

    if(state[SDL_SCANCODE_ESCAPE]) {
        controller_cmd(ctrl, ACT_ESC, ev);
    }
}
