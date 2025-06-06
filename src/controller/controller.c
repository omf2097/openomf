#include "controller/controller.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdlib.h>

#define DELAY_BUFFER_SIZE 11

typedef struct {
    void (*fp)(controller *ctrl, int act_type);
    controller *source;
} hook_function;

struct event_buffer_element {
    uint32_t tick;
    uint8_t actions[10];
};

void controller_init(controller *ctrl, game_state *gs) {
    list_create(&ctrl->hooks);
    ctrl->buffer = NULL;
    ctrl->gs = gs;
    ctrl->extra_events = NULL;
    ctrl->har_obj_id = 0;
    ctrl->poll_fun = NULL;
    ctrl->tick_fun = NULL;
    ctrl->dyntick_fun = NULL;
    ctrl->har_hook = NULL;
    ctrl->rumble_fun = NULL;
    ctrl->rtt = 0;
    ctrl->repeat = 0;
    ctrl->delay = 0;
    ctrl->supports_delay = false;
}

void controller_add_hook(controller *ctrl, controller *source, void (*fp)(controller *ctrl, int act_type)) {
    hook_function *h = omf_calloc(1, sizeof(hook_function));
    h->fp = fp;
    h->source = source;
    list_append(&ctrl->hooks, &h, sizeof(hook_function *));
}

void controller_clear_hooks(controller *ctrl) {
    iterator it;
    hook_function **tmp = 0;
    list_iter_begin(&ctrl->hooks, &it);
    foreach(it, tmp) {
        omf_free(*tmp);
        list_delete(&ctrl->hooks, &it);
    }
}

void controller_free_chain(ctrl_event *ev) {
    ctrl_event *now = ev;
    ctrl_event *tmp;
    while(now != NULL) {
        tmp = now->next;
        omf_free(now);
        now = tmp;
    }
}

void controller_free(controller *ctrl) {
    controller_clear_hooks(ctrl);
    list_free(&ctrl->hooks);
    if(ctrl->buffer) {
        vector_free(ctrl->buffer);
        omf_free(ctrl->buffer);
    }
    ctrl->free_fun(ctrl);
}

static inline void ctrl_action_push(ctrl_event **ev, int action) {
    ctrl_event *new = omf_calloc(1, sizeof(ctrl_event));

    new->type = EVENT_TYPE_ACTION;
    new->event_data.action = action;

    if(*ev == NULL) {
        *ev = new;
    } else {
        ctrl_event *i = *ev;
        while(i->next) {
            i = i->next;
        }
        i->next = new;
    }
}

void controller_cmd(controller *ctrl, int action, ctrl_event **ev) {
    ctrl->current |= action;

    // always debounce these actions
    action &= ~(ctrl->last & (ACT_KICK | ACT_PUNCH | ACT_ESC));

    if(!ctrl->repeat && (action & ACT_Mask_Dirs)) {
        // we're on a menu or something else that wants delayed keyrepeat for directions
        if(ctrl->repeat_tick == 0 || !(ctrl->last & ACT_Mask_Dirs)) {
            ctrl->repeat_tick = 30;
        } else {
            action &= ~ACT_Mask_Dirs;
        }
    }

    if(action == ACT_NONE) {
        // above debouncing probably ate our action
        action = ACT_STOP;
    }

    // fire any installed hooks *immediately*, no delay
    iterator it;
    hook_function **p = 0;
    list_iter_begin(&ctrl->hooks, &it);
    foreach(it, p) {
        hook_function hook = **p;
        (hook.fp)(hook.source, action);
    }

    if(ctrl->delay) {
        // enqueue delayed events
        struct event_buffer_element *buf =
            vector_get(ctrl->buffer, (ctrl->gs->int_tick + ctrl->delay) % DELAY_BUFFER_SIZE);
        if(buf->tick != ctrl->gs->int_tick + ctrl->delay) {
            // stale buffer element, zero it out
            memset(buf, 0, sizeof(struct event_buffer_element));
            buf->tick = ctrl->gs->int_tick + ctrl->delay;
        }
        for(int i = 0; i < 10; i++) {
            if(buf->actions[i] != 0) {
                continue;
            }
            if(i > 0 && buf->actions[i - 1] == action) {
                break;
            }
            buf->actions[i] = action;
        }

        // send any delayed events out
        buf = vector_get(ctrl->buffer, (ctrl->gs->int_tick) % DELAY_BUFFER_SIZE);
        if(buf->tick != ctrl->gs->int_tick) {
            return;
        }

        for(int i = 0; i < 10 && buf->actions[i] != 0; i++) {
            ctrl_action_push(ev, buf->actions[i]);
        }
        buf->tick = 0;
    } else {
        // no delay
        ctrl_action_push(ev, action);
    }
}

void controller_close(controller *ctrl, ctrl_event **ev) {
    // a close event obsoletes all previous events
    controller_free_chain(*ev);
    *ev = omf_calloc(1, sizeof(ctrl_event));
    (*ev)->type = EVENT_TYPE_CLOSE;
    (*ev)->next = NULL;
}

int controller_tick(controller *ctrl, uint32_t ticks, ctrl_event **ev) {
    if(ctrl->repeat_tick) {
        ctrl->repeat_tick--;
    }
    if(ctrl->tick_fun != NULL) {
        return ctrl->tick_fun(ctrl, ticks, ev);
    }
    return 0;
}

int controller_dyntick(controller *ctrl, uint32_t ticks, ctrl_event **ev) {
    if(ctrl->dyntick_fun != NULL) {
        return ctrl->dyntick_fun(ctrl, ticks, ev);
    }
    return 0;
}

int controller_poll(controller *ctrl, ctrl_event **ev) {
    if(ctrl->poll_fun != NULL) {
        return ctrl->poll_fun(ctrl, ev);
    }
    return 0;
}

int controller_har_hook(controller *ctrl, har_event event) {
    if(ctrl->har_hook != NULL) {
        return ctrl->har_hook(ctrl, event);
    }
    return 0;
}

void controller_set_repeat(controller *ctrl, int repeat) {
    ctrl->repeat = repeat;
}

bool controller_set_delay(controller *ctrl, uint8_t delay) {
    if(ctrl->supports_delay && delay > 0 && delay <= 10) {
        if(!ctrl->buffer) {
            ctrl->buffer = omf_calloc(1, sizeof(vector));
            vector_create_with_size(ctrl->buffer, sizeof(struct event_buffer_element), DELAY_BUFFER_SIZE);
            struct event_buffer_element buf;
            memset(&buf, 0, sizeof(struct event_buffer_element));
            for(int i = 0; i < DELAY_BUFFER_SIZE; i++) {
                vector_append(ctrl->buffer, &buf);
            }
        }
        ctrl->delay = delay;
        return true;
    }
    ctrl->delay = 0;
    return false;
}

int controller_rumble(controller *ctrl, float magnitude, int duration) {
    if(ctrl->rumble_fun != NULL) {
        return ctrl->rumble_fun(ctrl, magnitude, duration);
    }
    return 0;
}

void controller_rewind(controller *ctrl) {
    ctrl->current = 0;
    if(ctrl->rewind_fun != NULL) {
        ctrl->rewind_fun(ctrl);
    }
}
