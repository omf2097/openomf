#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <SDL2/SDL.h>
#include "game/protos/object.h"
#include "game/objects/har.h"
#include "game/serial.h"
#include "utils/list.h"

enum {
    ACT_KICK,
    ACT_PUNCH,
    ACT_UP,
    ACT_UPLEFT,
    ACT_UPRIGHT,
    ACT_DOWN,
    ACT_DOWNLEFT,
    ACT_DOWNRIGHT,
    ACT_LEFT,
    ACT_RIGHT,
    ACT_STOP,
    ACT_WALKLEFT,
    ACT_WALKRIGHT,
    ACT_CROUCH,
    ACT_JUMP
};

enum {
    CTRL_TYPE_KEYBOARD,
    CTRL_TYPE_GAMEPAD,
    CTRL_TYPE_NETWORK,
};

enum {
    EVENT_TYPE_ACTION,
    EVENT_TYPE_SYNC,
    EVENT_TYPE_CLOSE
};

typedef struct ctrl_event_t ctrl_event;

struct ctrl_event_t {
    int type;
    union {
        int action;
        serial *ser;
    } event_data;
    ctrl_event *next;
};

typedef struct controller_t controller;

struct controller_t {
    object *har;
    list hooks;
    ctrl_event *extra_events;
    int (*tick_fun)(controller *ctrl, int ticks, ctrl_event **ev);
    int (*poll_fun)(controller *ctrl, ctrl_event **ev);
    int (*event_fun)(controller *ctrl, SDL_Event *event, ctrl_event **ev);
    int (*update_fun)(controller *ctrl, serial *state);
    void (*har_hook)(char* buf, void *userdata);
    void (*controller_hook)(controller *ctrl, int action);
    void *data;
    int type;
};

void controller_init(controller* ctrl);
void controller_cmd(controller* ctrl, int action, ctrl_event **ev);
void controller_sync(controller *ctrl, serial *ser, ctrl_event **ev);
void controller_close(controller* ctrl, ctrl_event **ev);
int controller_event(controller *ctrl, SDL_Event *event, ctrl_event **ev);
int controller_poll(controller *ctrl, ctrl_event **ev);
int controller_tick(controller *ctrl, int ticks, ctrl_event **ev);
int controller_update(controller *ctrl, serial *state);
void controller_add_hook(controller *ctrl, controller *source, void(*fp)(controller *ctrl, int act_type));
void controller_clear_hooks(controller *ctrl);
void controller_free_chain(ctrl_event *ev);

#endif // _CONTROLLER_H
