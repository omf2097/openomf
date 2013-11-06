#include <stdlib.h>
#include "game/objects/projectile.h"
#include "utils/log.h"

void projectile_tick(object *obj) {
    //projectile_local *local = object_get_userdata(obj);
    // TODO: Implement this
}

void projectile_free(object *obj) {
    free(object_get_userdata(obj));
}

void projectile_move(object *obj) {
    obj->pos.x += obj->vel.x;
    obj->pos.y += obj->vel.y;
    
    // If projectile hits the wall, kill it
    // TODO: Seldct correct dispersal animation
    if(obj->pos.x < 10) {
        obj->pos.x = 10;
        obj->animation_state.finished = 1;
    }
    if(obj->pos.x > 310) {
        obj->pos.x = 310;
        obj->animation_state.finished = 1;
    }
}

int projectile_create(object *obj) {
    projectile_local *local = malloc(sizeof(projectile_local));
    object_set_userdata(obj, local);
    local->_empty = 0;

    object_set_tick_cb(obj, projectile_tick);
    object_set_free_cb(obj, projectile_free);
    object_set_move_cb(obj, projectile_move);

    return 0;
}
