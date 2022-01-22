#include "controller/controller.h"
#include "utils/allocator.h"
#include <stdlib.h>

typedef struct hook_function_t {
    void (*fp)(controller *ctrl, int act_type);
    controller *source;
} hook_function;

void controller_init(controller *ctrl) {
    list_create(&ctrl->hooks);
    ctrl->extra_events = NULL;
    ctrl->har = NULL;
    ctrl->poll_fun = NULL;
    ctrl->tick_fun = NULL;
    ctrl->dyntick_fun = NULL;
    ctrl->update_fun = NULL;
    ctrl->har_hook = NULL;
    ctrl->rumble_fun = NULL;
    ctrl->rtt = 0;
    ctrl->repeat = 0;
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
    while((tmp = iter_next(&it)) != NULL) {
        omf_free(*tmp);
        list_delete(&ctrl->hooks, &it);
    }
}

void controller_free_chain(ctrl_event *ev) {
    ctrl_event *now = ev;
    ctrl_event *tmp;
    while(now != NULL) {
        if(now->type == EVENT_TYPE_SYNC) {
            serial_free(now->event_data.ser);
            omf_free(now->event_data.ser);
        }
        tmp = now->next;
        omf_free(now);
        now = tmp;
    }
}

void controller_cmd(controller *ctrl, int action, ctrl_event **ev) {
    // fire any installed hooks
    iterator it;
    hook_function **p = 0;
    ctrl_event *i;
    ctrl_event *new;

    list_iter_begin(&ctrl->hooks, &it);
    while((p = iter_next(&it)) != NULL) {
        ((*p)->fp)((*p)->source, action);
    }

    new = omf_calloc(1, sizeof(ctrl_event));
    new->type = EVENT_TYPE_ACTION;
    new->event_data.action = action;

    if(*ev == NULL) {
        *ev = new;
    } else {
        i = *ev;
        while(i->next) {
            i = i->next;
        }
        i->next = new;
    }
}

void controller_sync(controller *ctrl, const serial *ser, ctrl_event **ev) {
    // a sync event obsoletes all previous events
    controller_free_chain(*ev);
    *ev = omf_calloc(1, sizeof(ctrl_event));
    (*ev)->type = EVENT_TYPE_SYNC;
    (*ev)->event_data.ser = serial_calloc_copy(ser);
    (*ev)->next = NULL;
}

void controller_close(controller *ctrl, ctrl_event **ev) {
    // a close event obsoletes all previous events
    controller_free_chain(*ev);
    *ev = omf_calloc(1, sizeof(ctrl_event));
    (*ev)->type = EVENT_TYPE_CLOSE;
    (*ev)->next = NULL;
}

int controller_tick(controller *ctrl, int ticks, ctrl_event **ev) {
    if(ctrl->tick_fun != NULL) {
        return ctrl->tick_fun(ctrl, ticks, ev);
    }
    return 0;
}

int controller_dyntick(controller *ctrl, int ticks, ctrl_event **ev) {
    if(ctrl->dyntick_fun != NULL) {
        return ctrl->dyntick_fun(ctrl, ticks, ev);
    }
    return 0;
}

int controller_update(controller *ctrl, serial *state) {
    if(ctrl->update_fun != NULL) {
        return ctrl->update_fun(ctrl, state);
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

int controller_rumble(controller *ctrl, float magnitude, int duration) {
    if(ctrl->rumble_fun != NULL) {
        return ctrl->rumble_fun(ctrl, magnitude, duration);
    }
    return 0;
}
