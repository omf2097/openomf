#include <stdlib.h>
#include "game/objects/projectile.h"
#include "game/objects/har.h"
#include "utils/log.h"

typedef struct projectile_local_t {
    har *har;
} projectile_local;

void projectile_tick(object *obj) {
    projectile_local *local = object_get_userdata(obj);
    har *prog_owner = local->har;

    if(obj->animation_state.finished) {
        af_move *move = af_get_move(&(prog_owner->af_data), obj->cur_animation->id);
        if (move->successor_id) {
            object_set_animation(obj, &af_get_move(&prog_owner->af_data, move->successor_id)->ani);
            object_set_repeat(obj, 0);
            object_set_vel(obj, vec2f_create(0,0));
            obj->animation_state.finished = 0;
        }
    }
}

void projectile_free(object *obj) {
    free(object_get_userdata(obj));
}

void projectile_move(object *obj) {
    obj->pos.x += obj->vel.x;
    obj->pos.y += obj->vel.y + obj->gravity;
    
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
    if(obj->pos.y > 190) {
        obj->pos.y = 190;
    }
}

int projectile_create(object *obj) {
    projectile_local *local = malloc(sizeof(projectile_local));
    // strore the HAR in here instead
    local->har = object_get_userdata(obj);
    object_set_userdata(obj, local);

    object_set_tick_cb(obj, projectile_tick);
    object_set_free_cb(obj, projectile_free);
    object_set_move_cb(obj, projectile_move);

    return 0;
}

har *projectile_get_har(object *obj) {
    return ((projectile_local*)object_get_userdata(obj))->har;
}
