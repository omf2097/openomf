#include <stdlib.h>
#include "game/objects/scrap.h"

#define SCRAP_KEEPALIVE 80
#define IS_ZERO(n) (n < 0.1 && n > -0.1)

void scrap_tick(object *obj) {
	scrap_local *local = object_get_userdata(obj);

	// Kill scrap animation, if it has been running long enough
	local->tick++;
	if(local->tick > SCRAP_KEEPALIVE) {
		obj->animation_state.finished = 1;
	}
}

void scrap_free(object *obj) {
	free(object_get_userdata(obj));
}

// TODO: This is kind of quick and dirty, think of something better.
void scrap_move(object *obj) {
    vec2f vel = object_get_vel(obj);
    vec2i pos = object_get_pos(obj);
    if(object_get_halt(obj)) {
    	return;
    }

    pos.x += vel.x;
    vel.y += obj->gravity;
    pos.y += vel.y;

    float dampen = 0.4;

    if(pos.x <  15) pos.x = 15;
    if(pos.x > 305) pos.x = 305;
    if(pos.y > 190) {
    	pos.y = 190;
    	vel.y = -vel.y * dampen;
    	vel.x = vel.x * dampen;
    }
    if(IS_ZERO(vel.x)) vel.x = 0;
    if(IS_ZERO(vel.y)) vel.y = 0;
    object_set_pos(obj, pos);
    object_set_vel(obj, vel);

    // If object is at rest, just halt animation
    if(IS_ZERO(vel.x) && vel.y < obj->gravity * 1.1 && vel.y > obj->gravity * -1.1) {
    	object_set_halt(obj, 1);
    }
}

int scrap_create(object *obj) {
	scrap_local *local = malloc(sizeof(scrap_local));
	object_set_userdata(obj, local);
	local->tick = 0;

	object_set_tick_cb(obj, scrap_tick);
	object_set_free_cb(obj, scrap_free);
	object_set_move_cb(obj, scrap_move);

	return 0;
}
