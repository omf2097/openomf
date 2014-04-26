#include <stdlib.h>
#include "game/objects/projectile.h"
#include "game/protos/object_specializer.h"
#include "game/game_state.h"
#include "game/game_player.h"
#include "utils/log.h"
#include "game/objects/arena_constraints.h"

typedef struct projectile_local_t {
    object *owner;
    af *af_data;
} projectile_local;

void projectile_tick(object *obj) {
    projectile_local *local = object_get_userdata(obj);

    if(obj->animation_state.finished) {
        af_move *move = af_get_move(local->af_data, obj->cur_animation->id);
        if (move->successor_id) {
            object_set_animation(obj, &af_get_move(local->af_data, move->successor_id)->ani);
            object_set_repeat(obj, 0);
            object_set_vel(obj, vec2f_create(0,0));
            obj->animation_state.finished = 0;
        }
    }

    // Note! If we ever add more effects, this will need to be changed!
    // XXX: Make this better.
    if(player_frame_isset(obj, "bt")) {
        object_set_effects(obj, EFFECT_DARK_TINT);
    } else {
        object_set_effects(obj, EFFECT_NONE);
    }
}

void projectile_free(object *obj) {
    free(object_get_userdata(obj));
}

void projectile_move(object *obj) {
    obj->pos.x += obj->vel.x;
    obj->vel.y += obj->gravity;
    obj->pos.y += obj->vel.y;

    float dampen = 0.7f;

    // If projectile hits the wall, kill it
    if(obj->pos.x < ARENA_LEFT_WALL) {
        obj->pos.x = ARENA_LEFT_WALL;
        obj->animation_state.finished = 1;
    }
    if(obj->pos.x > ARENA_RIGHT_WALL) {
        obj->pos.x = ARENA_RIGHT_WALL;
        obj->animation_state.finished = 1;
    }
    if(obj->pos.y > ARENA_FLOOR) {
        obj->pos.y = ARENA_FLOOR;
        obj->vel.y = -obj->vel.y * dampen;
    }
}

int projectile_serialize(object *obj, serial *ser) {
    projectile_local *local = object_get_userdata(obj);
    serial_write_int8(ser, SPECID_PROJECTILE);
    serial_write_int8(ser, local->af_data->id);
    return 0;
}

int projectile_unserialize(object *obj, serial *ser, int animation_id, game_state *gs) {
    uint8_t har_id = serial_read_int8(ser);

    game_player *player;
    object *o;
    har *h;

    for(int i = 0; i < 2; i++) {
        player = game_state_get_player(gs, i);
        o = game_player_get_har(player);
        h = object_get_userdata(o);
        if (h->af_data->id == har_id) {
            af_move *move = af_get_move(h->af_data, animation_id);
            object_set_animation(obj, &move->ani);
            object_set_userdata(obj, h);
            object_set_stl(obj, object_get_stl(o));
            projectile_create(obj);
            return 0;
        }
    }
    DEBUG("COULD NOT FIND HAR ID %d", har_id);
    // could not find the right HAR...
    return 1;
}

void projectile_bootstrap(object *obj) {
    object_set_serialize_cb(obj, projectile_serialize);
    object_set_unserialize_cb(obj, projectile_unserialize);
}

int projectile_create(object *obj) {
    projectile_local *local = malloc(sizeof(projectile_local));
    // strore the HAR in here instead
    local->owner = obj;
    local->af_data = ((har*)object_get_userdata(obj))->af_data;
    object_set_userdata(obj, local);

    object_set_dynamic_tick_cb(obj, projectile_tick);
    object_set_free_cb(obj, projectile_free);
    object_set_move_cb(obj, projectile_move);

    projectile_bootstrap(obj);

    return 0;
}

af *projectile_get_af_data(object *obj) {
    return ((projectile_local*)object_get_userdata(obj))->af_data;
}

object *projectile_get_owner(object *obj) {
    return ((projectile_local*)object_get_userdata(obj))->owner;
}

