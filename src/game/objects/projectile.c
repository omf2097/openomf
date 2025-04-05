#include "game/objects/projectile.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/objects/arena_constraints.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdlib.h>

#define IS_ZERO(n) (n < 0.1 && n > -0.1)

typedef struct projectile_local_t {
    uint8_t player_id;
    const af *af_data;
    int wall_bounce;
    int ground_freeze;
    int invincible;
    bool has_hit;
    uint32_t linked_obj;
    uint32_t parent_id;
} projectile_local;

void projectile_finished(object *obj) {
    projectile_local *local = object_get_userdata(obj);

    if(local->linked_obj) {
        object *linked = game_state_find_object(obj->gs, local->linked_obj);
        if(linked) {
            linked->animation_state.disable_d = 1;
        }
    }
    af_move *move = af_get_move(local->af_data, obj->cur_animation->id);
    if(move->successor_id) {
        object_set_animation(obj, &af_get_move(local->af_data, move->successor_id)->ani);
        object_set_repeat(obj, 0);
        object_set_vel(obj, vec2f_create(0, 0));
        obj->animation_state.finished = 0;
    }
}

void projectile_free(object *obj) {
    projectile_local *local = object_get_userdata(obj);
    omf_free(local);
    object_set_userdata(obj, NULL);
}

void projectile_move(object *obj) {
    projectile_local *local = object_get_userdata(obj);

    game_state *gs = obj->gs;
    game_player *player = game_state_get_player(gs, projectile_get_owner(obj));
    object *obj_har = game_state_find_object(gs, game_player_get_har_obj_id(player));

    obj->pos.x += obj->vel.x;
    obj->vel.y += obj->gravity;
    obj->pos.y += obj->vel.y;

    float dampen = 0.7f;

    // If wall bounce flag is on, bounce the projectile on wall hit
    // Otherwise kill it.
    if(local->wall_bounce) {
        if(obj->pos.x < ARENA_LEFT_WALL) {
            obj->pos.x = ARENA_LEFT_WALL;
            obj->vel.x = -obj->vel.x * dampen;
        }
        if(obj->pos.x > ARENA_RIGHT_WALL) {
            obj->pos.x = ARENA_RIGHT_WALL;
            obj->vel.x = -obj->vel.x * dampen;
        }
        // if not invincible, not ignoring bounds checking and actually has an X velocity (the latter two help with
        // shadow grab)
    } else if(!local->invincible && !player_frame_isset(obj, "bh") && !IS_ZERO(obj->vel.x)) {
        if(obj->pos.x < ARENA_LEFT_WALL) {
            obj->pos.x = ARENA_LEFT_WALL;
            obj->animation_state.finished = 1;
            projectile_finished(obj);
        }
        if(obj->pos.x > ARENA_RIGHT_WALL) {
            obj->pos.x = ARENA_RIGHT_WALL;
            obj->animation_state.finished = 1;
            projectile_finished(obj);
        }
    }
    if(obj->pos.y > ARENA_FLOOR && local->wall_bounce) {
        obj->pos.y = ARENA_FLOOR;
        obj->vel.y = -obj->vel.y * dampen;
        obj->vel.x = obj->vel.x * dampen;
    } else if(obj->pos.y > ARENA_FLOOR) {
        obj->pos.y = ARENA_FLOOR;
        obj->animation_state.finished = 1;
        projectile_finished(obj);
    }
    if(obj->pos.y >= (ARENA_FLOOR - 5) && IS_ZERO(obj->vel.x) && obj->vel.y < obj->gravity * 1.1 &&
       obj->vel.y > obj->gravity * -1.1 && local->ground_freeze) {

        object_disable_rewind_tag(obj, 1);
    }
    har *h = object_get_userdata(obj_har);
    object_apply_controllable_velocity(obj, true, h->inputs[0]);
}

int projectile_clone(object *src, object *dst) {
    projectile_local *local = omf_calloc(1, sizeof(projectile_local));
    memcpy(local, object_get_userdata(src), sizeof(projectile_local));
    object_set_userdata(dst, local);

    object *old_har_obj = dst->animation_state.disable_userdata;

    object *har_obj = game_state_find_object(dst->gs, old_har_obj->id);

    object_set_spawn_cb(dst, cb_har_spawn_object, har_obj);
    object_set_disable_cb(dst, cb_har_disable_animation, har_obj);

    return 0;
}

int projectile_clone_free(object *obj) {
    projectile_local *local = object_get_userdata(obj);
    omf_free(local);
    object_set_userdata(obj, NULL);
    return 0;
}

int projectile_create(object *obj, object *parent) {
    har *har = object_get_userdata(parent);
    // strore the HAR in local userdata instead
    projectile_local *local = omf_calloc(1, sizeof(projectile_local));
    local->player_id = har->player_id;
    local->wall_bounce = 0;
    local->ground_freeze = 0;
    local->af_data = har->af_data;
    local->has_hit = false;
    local->parent_id = parent->id;

    // Set up callbacks
    object_set_userdata(obj, local);
    object_set_free_cb(obj, projectile_free);
    object_set_move_cb(obj, projectile_move);
    object_set_finish_cb(obj, projectile_finished);
    obj->clone = projectile_clone;
    obj->clone_free = projectile_clone_free;
    return 0;
}

const af *projectile_get_af_data(object *obj) {
    return ((projectile_local *)object_get_userdata(obj))->af_data;
}

uint8_t projectile_get_owner(object *obj) {
    return ((projectile_local *)object_get_userdata(obj))->player_id;
}

void projectile_set_wall_bounce(object *obj, int bounce) {
    projectile_local *local = object_get_userdata(obj);
    local->wall_bounce = bounce;
}

void projectile_set_invincible(object *obj) {
    projectile_local *local = object_get_userdata(obj);
    local->invincible = 1;
}

void projectile_stop_on_ground(object *obj, int stop) {
    projectile_local *local = object_get_userdata(obj);
    local->ground_freeze = stop;
}

void projectile_mark_hit(object *obj) {
    projectile_local *local = object_get_userdata(obj);
    local->has_hit = true;
}

bool projectile_did_hit(object *obj) {
    projectile_local *local = object_get_userdata(obj);
    return local->has_hit;
}

void projectile_clear_hit(object *obj) {
    projectile_local *local = object_get_userdata(obj);
    local->has_hit = false;
}

void projectile_link_object(object *obj, object *link) {
    projectile_local *local = object_get_userdata(obj);
    local->linked_obj = link->id;
}

void projectile_connect_to_parent(object *obj) {
    projectile_local *local = object_get_userdata(obj);
    object *parent = game_state_find_object(obj->gs, local->parent_id);
    if(parent) {
        har_connect_child(parent, obj);
    }
}
