#include <stdlib.h>
#include "utils/log.h"
#include "controller/controller.h"

typedef struct hook_function_t {
    void(*fp)(controller *ctrl, int act_type);
    controller *source;
} hook_function;

void controller_init(controller *ctrl) {
    list_create(&ctrl->hooks);
    ctrl->har = NULL;
    ctrl->handle_fun = NULL;
    ctrl->tick_fun = NULL;
}

void controller_add_hook(controller *ctrl, controller *source, void(*fp)(controller *ctrl, int act_type)) {
    hook_function *h = malloc(sizeof(hook_function));
    h->fp = fp;
    h->source = source;
    list_append(&ctrl->hooks, &h, sizeof(hook_function*));
}

void controller_clear_hooks(controller *ctrl) {
    iterator it;
    hook_function **tmp = 0;
    list_iter_begin(&ctrl->hooks, &it);
    while((tmp = iter_next(&it)) != NULL) {
        free(*tmp);
        list_delete(&ctrl->hooks, &it); 
    }
}


void controller_cmd(controller* ctrl, int action, ctrl_event **ev) {
    // fire any installed hooks
    iterator it;
    hook_function **p = 0;
    ctrl_event *i;
    
    list_iter_begin(&ctrl->hooks, &it);
    while((p = iter_next(&it)) != NULL) {
        ((*p)->fp)((*p)->source, action);
    }
    if (*ev == NULL) {
        *ev = malloc(sizeof(ctrl_event));
        (*ev)->action = action;
        (*ev)->next = NULL;
    } else {
        i = *ev;
        while (i->next) {}
        i->next = malloc(sizeof(ctrl_event));
        i->next->action = action;
        i->next->next = NULL;
    }
}

int controller_event(controller *ctrl, SDL_Event *event, ctrl_event **ev) {
    if(ctrl->handle_fun != NULL) {
        return ctrl->handle_fun(ctrl, event, ev);
    }
    return 0;
}

int controller_tick(controller *ctrl, ctrl_event **ev) {
    if(ctrl->tick_fun != NULL) {
        return ctrl->tick_fun(ctrl, ev);
    }
    return 0;
}
