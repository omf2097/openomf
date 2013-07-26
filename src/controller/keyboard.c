#include "controller/keyboard.h"
#include "utils/log.h"

void keyboard_create(controller *ctrl, har *har, keyboard_keys *keys) {
    keyboard *k = malloc(sizeof(keyboard));
    k->keys = keys;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->har = har;
}

void keyboard_free(controller *ctrl) {
    keyboard *k = ctrl->data;
    free(k->keys);
    free(k);
}

void keyboard_tick(controller *ctrl) {
    keyboard *k = ctrl->data;
    const unsigned char *state = SDL_GetKeyboardState(NULL);
    int handled = 0;

    if (state[k->keys->down]) {
        har_act(ctrl->har, ACT_CROUCH);
        handled = 1;
    }
    if (state[k->keys->up]) {
        har_act(ctrl->har, ACT_JUMP);
        handled = 1;
    }
    if(state[k->keys->right] && !state[k->keys->left] &&
            !state[k->keys->down] && !state[k->keys->kick] && !state[k->keys->punch]) {
        har_act(ctrl->har, ACT_WALKRIGHT);
        handled = 1;
    }
    if(state[k->keys->left] && !state[k->keys->right] &&
            !state[k->keys->down] && !state[k->keys->kick] && !state[k->keys->punch]) {
        har_act(ctrl->har, ACT_WALKLEFT);
        handled = 1;
    }
    if(!handled) {
        har_act(ctrl->har, ACT_STOP);
    }
}

int keyboard_handle(controller *ctrl, SDL_Event *event) {
    keyboard *k = ctrl->data;
    if (event->type == SDL_KEYUP && (event->key.keysym.sym == k->keys->kick || event->key.keysym.sym == k->keys->punch)) {
        return 1;
    }
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        const unsigned char *state = SDL_GetKeyboardState(NULL);
        if ( state[k->keys->left] && state[k->keys->up]) {
            har_act(ctrl->har, ACT_UPLEFT);
        } else if ( state[k->keys->left] && state[k->keys->down]) {
            har_act(ctrl->har, ACT_DOWNLEFT);
        } else  if ( state[k->keys->right] && state[k->keys->up]) {
            har_act(ctrl->har, ACT_UPRIGHT);
        } else  if ( state[k->keys->right] && state[k->keys->down]) {
            har_act(ctrl->har, ACT_DOWNRIGHT);
        } else if ( state[k->keys->right]) {
            har_act(ctrl->har, ACT_RIGHT);
        } else if ( state[k->keys->left]) {
            har_act(ctrl->har, ACT_LEFT);
        } else if ( state[k->keys->up]) {
            har_act(ctrl->har, ACT_UP);
        } else if ( state[k->keys->down]) {
            har_act(ctrl->har, ACT_DOWN);
        }

        if (state[k->keys->punch]) {
            har_act(ctrl->har, ACT_PUNCH);
        } else if (state[k->keys->kick]) {
            har_act(ctrl->har, ACT_KICK);
        }
        return 0;
    }
    // If key was handled here, return 0. Otherwise 1.
    return 1;
}
