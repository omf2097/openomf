#include "game/objects/scrap.h"
#include "game/objects/arena_constraints.h"
#include <stdlib.h>

#define SCRAP_KEEPALIVE 220
#define IS_ZERO(n) (n < 0.1 && n > -0.1)

// TODO: This is kind of quick and dirty, think of something better.
void scrap_move(object *obj) {
    vec2f vel = object_get_vel(obj);
    vec2i pos = object_get_pos(obj);
    if (object_is_rewind_tag_disabled(obj) > 0) {
        return;
    }

    pos.x += vel.x;
    vel.y += obj->gravity;
    pos.y += vel.y;

    float dampen = 0.4;

    if (pos.x < ARENA_LEFT_WALL) {
        pos.x = ARENA_LEFT_WALL;
        vel.x = -vel.x * dampen;
    }
    if (pos.x > ARENA_RIGHT_WALL) {
        pos.x = ARENA_RIGHT_WALL;
        vel.x = -vel.x * dampen;
    }
    if (pos.y > ARENA_FLOOR) {
        pos.y = ARENA_FLOOR;
        vel.y = -vel.y * dampen;
        vel.x = vel.x * dampen;
    }
    if (IS_ZERO(vel.x))
        vel.x = 0;
    object_set_pos(obj, pos);
    object_set_vel(obj, vel);

    // If object is at rest, just halt animation
    if (pos.y >= (ARENA_FLOOR - 5) && IS_ZERO(vel.x) && vel.y < obj->gravity * 1.1 &&
        vel.y > obj->gravity * -1.1) {
        object_disable_rewind_tag(obj, 1);
    }
}

int scrap_create(object *obj) {
    object_set_move_cb(obj, scrap_move);

    return 0;
}
