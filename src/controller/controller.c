#include <stdlib.h>
#include "utils/log.h"
#include "controller/controller.h"

typedef struct hook_function_t {
    void(*fp)(controller *ctrl, int act_type);
    controller *source;
} hook_function;

void controller_init(controller *ctrl) {
    list_create(&ctrl->hooks);
}

void controller_add_hook(controller *ctrl, controller *source, void(*fp)(controller *ctrl, int act_type)) {
    hook_function *h = malloc(sizeof(hook_function));
    h->fp = fp;
    h->source = source;
    list_append(&ctrl->hooks, &h, sizeof(hook_function*));
}

void controller_cmd(controller* ctrl, int action) {
    // fire any installed hooks
    iterator it;
    hook_function **p = 0;
    
    if (action != 10) {
        DEBUG("controller sent action %d to HAR %p", action, ctrl->har);
    }
    list_iter_begin(&ctrl->hooks, &it);
    while((p = iter_next(&it)) != NULL) {
        ((*p)->fp)((*p)->source, action);
    }
    har_act(ctrl->har, action);
}

int controller_event(controller *ctrl, SDL_Event *event) {
    return ctrl->handle_fun(ctrl, event);
}

void controller_tick(controller *ctrl) {
    ctrl->tick_fun(ctrl);
}
