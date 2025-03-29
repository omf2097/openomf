#include "game/objects/scrap.h"
#include "game/objects/arena_constraints.h"

#define SCRAP_KEEPALIVE 220
#define IS_ZERO(n) (n < fixedpt_rconst(0.1) && n > fixedpt_rconst(-0.1))

// TODO: This is kind of quick and dirty, think of something better.
void scrap_move(object *obj) {
    vec2f vel = object_get_vel(obj);
    vec2f pos = obj->pos;
    if(object_is_rewind_tag_disabled(obj) > 0) {
        return;
    }

    pos.fx += vel.fx;
    vel.fy += obj->gravity;
    pos.fy += vel.fy;

#define dampen 4 / 10

    if(pos.fx < ARENA_LEFT_WALLF) {
        pos.fx = ARENA_LEFT_WALLF;
        vel.fx = -vel.fx * dampen;
    }
    if(pos.fx > ARENA_RIGHT_WALLF) {
        pos.fx = ARENA_RIGHT_WALLF;
        vel.fx = -vel.fx * dampen;
    }
    if(pos.fy > ARENA_FLOORF) {
        pos.fy = ARENA_FLOORF;
        vel.fy = -vel.fy * dampen;
        vel.fx = vel.fx * dampen + (rand_float() - 0.5f) * 3.0;
    }
    if(IS_ZERO(vel.fx))
        vel.fx = 0;
    obj->pos = pos;
    object_set_vel(obj, vel);

    // If object is at rest, just halt animation
    if(pos.fy >= (ARENA_FLOORF - fixedpt_fromint(5)) && IS_ZERO(vel.fx) && vel.fy < obj->gravity * 11 / 10 &&
       vel.fy > obj->gravity * -11 / 10) {
        object_disable_rewind_tag(obj, 1);
    }
}

int scrap_create(object *obj) {
    object_set_move_cb(obj, scrap_move);

    return 0;
}
