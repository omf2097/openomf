#include "controller/keyboard.h"
#include "utils/log.h"
#include <stdlib.h>

void keyboard_free(controller *ctrl) {
    keyboard *k = ctrl->data;
    free(k->keys);
    free(k);
}

int keyboard_tick(controller *ctrl, int ticks, ctrl_event **ev) {
    keyboard *k = ctrl->data;
    k->tick = ticks;
    if(k->queued == KEYBOARD_INPUT_BUFFER_SIZE) {
        return 0;
    }
    int i = k->queued;
    k->queued++;
    const unsigned char *state = SDL_GetKeyboardState(NULL);
    k->buffer[i] = ACT_STOP;

    if ( state[k->keys->left] && state[k->keys->up]) {
        k->buffer[i] |= ACT_UPLEFT;
    } else if ( state[k->keys->left] && state[k->keys->down]) {
        k->buffer[i] |= ACT_DOWNLEFT;
    } else  if ( state[k->keys->right] && state[k->keys->up]) {
        k->buffer[i] |= ACT_UPRIGHT;
    } else  if ( state[k->keys->right] && state[k->keys->down]) {
        k->buffer[i] |= ACT_DOWNRIGHT;
    } else if ( state[k->keys->right]) {
        k->buffer[i] |= ACT_RIGHT;
    } else if ( state[k->keys->left]) {
        k->buffer[i] |= ACT_LEFT;
    } else if ( state[k->keys->up]) {
        k->buffer[i] |= ACT_UP;
    } else if ( state[k->keys->down]) {
        k->buffer[i] |= ACT_DOWN;
    }

    if (state[k->keys->punch]) {
        k->buffer[i] |= ACT_PUNCH;
    } else if (state[k->keys->kick]) {
        k->buffer[i] |= ACT_KICK;
    }

    return 0;
}

int keyboard_poll(controller *ctrl, ctrl_event **ev) {
    keyboard *k = ctrl->data;
    int i = 0;
    while(k->queued > 0) {
        uint16_t input = k->buffer[i];
        if (input & ACT_UPLEFT) {
            controller_cmd(ctrl, ACT_UPLEFT, ev);
        } else if (input & ACT_DOWNLEFT) {
            controller_cmd(ctrl, ACT_DOWNLEFT, ev);
        } else  if (input & ACT_UPRIGHT) {
            controller_cmd(ctrl, ACT_UPRIGHT, ev);
        } else  if (input & ACT_DOWNRIGHT) {
            controller_cmd(ctrl, ACT_DOWNRIGHT, ev);
        } else if (input & ACT_RIGHT) {
            controller_cmd(ctrl, ACT_RIGHT, ev);
        } else if (input & ACT_LEFT) {
            controller_cmd(ctrl, ACT_LEFT, ev);
        } else if (input & ACT_UP) {
            controller_cmd(ctrl, ACT_UP, ev);
        } else if (input & ACT_DOWN) {
            controller_cmd(ctrl, ACT_DOWN, ev);
        }

        if (input & ACT_PUNCH) {
            controller_cmd(ctrl, ACT_PUNCH, ev);
        } else if (input & ACT_KICK) {
            controller_cmd(ctrl, ACT_KICK, ev);
        }

        if(input == ACT_STOP) {
            controller_cmd(ctrl, ACT_STOP, ev);
        }
        k->queued--;
        i++;
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

void keyboard_create(controller *ctrl, keyboard_keys *keys, int delay) {
    keyboard *k = malloc(sizeof(keyboard));
    k->keys = keys;
    k->delay = delay;
    k->queued = 0;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->poll_fun = &keyboard_poll;
    ctrl->event_fun = &keyboard_event;
    ctrl->tick_fun = &keyboard_tick;
    for (int i = 0; i < KEYBOARD_INPUT_BUFFER_SIZE; i++) {
        k->buffer[i] = ACT_STOP;
    }
    /*controller_add_hook(ctrl, &hook);*/
}

void keyboard_set_delay(controller *ctrl, int delay) {
    keyboard *k = ctrl->data;
    if (delay != k->delay && delay < 10 && delay > 0) {
        DEBUG("Keyboard input delay changed: %d -> %d", k->delay, delay);
        k->delay = delay;
    }
}
