#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "game/game_state_type.h"
#include "game/objects/har.h"
#include "game/protos/object.h"
#include "game/utils/serial.h"
#include "utils/list.h"

enum
{
    ACT_STOP = 0x01,
    ACT_KICK = 0x02,
    ACT_PUNCH = 0x04,
    ACT_UP = 0x08,
    ACT_DOWN = 0x10,
    ACT_LEFT = 0x20,
    ACT_ESC = 0x40,
    ACT_RIGHT = 0x80
};

enum
{
    CTRL_TYPE_KEYBOARD,
    CTRL_TYPE_GAMEPAD,
    CTRL_TYPE_NETWORK,
    CTRL_TYPE_AI,
    CTRL_TYPE_REC
};

enum
{
    EVENT_TYPE_ACTION,
    EVENT_TYPE_HB,
    EVENT_TYPE_PROPOSE_START,
    EVENT_TYPE_CONFIRM_START,
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
    game_state *gs;
    uint32_t har_obj_id;
    list hooks;
    ctrl_event *extra_events;
    int (*tick_fun)(controller *ctrl, int ticks, ctrl_event **ev);
    int (*dyntick_fun)(controller *ctrl, int ticks, ctrl_event **ev);
    int (*poll_fun)(controller *ctrl, ctrl_event **ev);
    int (*rumble_fun)(controller *ctrl, float magnitude, int duration);
    int (*har_hook)(controller *ctrl, har_event event);
    void (*controller_hook)(controller *ctrl, int action);
    void (*free_fun)(controller *ctrl);
    void *data;
    int type;
    int rtt;
    int repeat;
};

void controller_init(controller *ctrl, game_state *gs);
void controller_cmd(controller *ctrl, int action, ctrl_event **ev);
void controller_close(controller *ctrl, ctrl_event **ev);
int controller_poll(controller *ctrl, ctrl_event **ev);
int controller_tick(controller *ctrl, int ticks, ctrl_event **ev);
int controller_dyntick(controller *ctrl, int ticks, ctrl_event **ev);
int controller_har_hook(controller *ctrl, har_event event);
void controller_add_hook(controller *ctrl, controller *source, void (*fp)(controller *ctrl, int act_type));
void controller_clear_hooks(controller *ctrl);
void controller_free_chain(ctrl_event *ev);
void controller_free(controller *ctrl);
void controller_set_repeat(controller *ctrl, int repeat);
int controller_rumble(controller *ctrl, float magnitude, int duration);

#endif // CONTROLLER_H
