#include "game/objects/hazard.h"
#include "game/protos/scene.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include <math.h>
#include <stdlib.h>

static fixedpt const dest_threshold = fixedpt_fromint(5);
static fixedpt const speed_cap = fixedpt_fromint(3);
static int const recip_accel_scale = 16; // higher values = less acceleration

int orb_almost_there(vec2f a, vec2f b) {
    return vec2f_distsqr(a, b) < fixedpt_xmul(dest_threshold, dest_threshold);
}

void hazard_tick(object *obj) {
    scene *sc = obj->gs->sc;

    if(obj->animation_state.finished) {
        bk_info *anim = bk_get_info(sc->bk_data, obj->cur_animation->id);
        if(anim->chain_no_hit) {
            object_set_animation(obj, &bk_get_info(sc->bk_data, anim->chain_no_hit)->ani);
            object_set_repeat(obj, 0);
            obj->animation_state.finished = 0;
        }
    }
    if(obj->orbit) {
        obj->orbit_tick += MATH_PI / 32;
        if(obj->orbit_tick >= MATH_PI * 2) {
            obj->orbit_tick -= MATH_PI * 2;
        }
        if(orb_almost_there(obj->orbit_dest, obj->orbit_pos)) {
            // XXX come up with a better equation to randomize the destination
            obj->orbit_pos = obj->pos;
            obj->orbit_pos_vary = vec2f_create(0, 0);
            int limit = 10;
            do {
                obj->orbit_dest = vec2f_create(random_fixedpt(&obj->gs->rand, fixedpt_fromint(320)),
                                               random_fixedpt(&obj->gs->rand, fixedpt_fromint(200)));
                limit--;
            } while(limit > 0 && vec2f_distsqr(obj->orbit_dest, obj->orbit_pos) < fixedpt_fromint(80 * 80));
        }
    }
}

void hazard_spawn_cb(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g, void *userdata) {
    scene *sc = parent->gs->sc;

    // Get next animation
    bk_info *info = bk_get_info(sc->bk_data, id);
    if(info != NULL) {
        object *obj = omf_calloc(1, sizeof(object));
        object_create(obj, parent->gs, vec2i_add(pos, info->ani.start_pos), vec2f_create(0, 0));
        object_set_stl(obj, object_get_stl(parent));
        object_set_animation(obj, &info->ani);
        if(info->probability == 1) {
            object_set_repeat(obj, 1);
        }
        object_set_layers(obj, LAYER_HAZARD | LAYER_HAR);
        object_set_group(obj, GROUP_PROJECTILE);
        object_set_userdata(obj, object_get_userdata(parent));
        hazard_create(obj, sc);
        if(sc->bk_data->file_id == 128 && id == 14) {
            // XXX hack because we don't understand the ms and md tags
            // without this, the 'bullet damage' sprite in the desert spawns at 0,0
            obj->pos = parent->pos;
        }
        game_state_add_object(parent->gs, obj, RENDER_LAYER_BOTTOM, 0, 0);
    } else {
        log_debug("failed to spawn hazard child");
    }
}

vec2f generate_destination(object *obj) {
    vec2f old = obj->orbit_dest;
    vec2f new;
    do {
        new = vec2f_create(random_fixedpt(&obj->gs->rand, fixedpt_fromint(280)) + fixedpt_fromint(20),
                           random_fixedpt(&obj->gs->rand, fixedpt_fromint(160)) + fixedpt_fromint(20));
    } while(vec2f_distsqr(old, new) < fixedpt_fromint(100 * 100));
    return new;
}

void hazard_move(object *obj) {
    if(obj->orbit) {
        /*
        if (obj->pos.x > 280 || obj->pos.x < 40 || obj->pos.y < 40 || obj->pos.y > 160) {
            if (obj->vel.x > 0.1f) {
                obj->vel.x /= 2;
            }
            if (obj->vel.x > 0.1f) {
                obj->vel.y /= 2;
            }
        }
        if (obj->pos.x >= 320 || obj->pos.x <= 0) {
            obj->vel.x = 0.0f;
        }

        if (obj->pos.x >= 200 || obj->pos.x <= 0) {
            obj->vel.y = 0.0f;
        }*/

        if((obj->orbit_pos.fx - obj->pos.fx >= obj->orbit_pos.fx - obj->orbit_dest.fx) &&
           (obj->orbit_pos.fy - obj->pos.fy >= obj->orbit_pos.fy - obj->orbit_dest.fy)) {
            obj->orbit_pos.fx = obj->pos.fx;
            obj->orbit_pos.fy = obj->pos.fy;
            obj->orbit_dest = generate_destination(obj);
            log_debug("new position is %f, %f", fixedpt_tofloat(obj->orbit_dest.fx),
                      fixedpt_tofloat(obj->orbit_dest.fy));
        }

        // accelerate_orbit(obj);

        fixedpt x_dist = obj->orbit_dest.fx - obj->pos.fx;
        fixedpt y_dist = obj->orbit_dest.fy - obj->pos.fy;
        fixedpt bigger = fixedpt_max2(fixedpt_abs(x_dist), fixedpt_abs(y_dist));

        vec2f accel = vec2f_create(fixedpt_xdiv(x_dist, bigger) / recip_accel_scale,
                                   fixedpt_xdiv(y_dist, bigger) / recip_accel_scale);

        // log_debug("dist %f %f", fixedpt_tofloat(x_dist), fixedpt_tofloat(y_dist));
        // log_debug("accel %f %f", fixedpt_tofloat(accel.fx), fixedpt_tofloat(accel.fy));

        obj->vel = vec2f_add(obj->vel, accel);

        fixedpt speed = vec2f_mag(obj->vel);
        if(speed > speed_cap) {
            obj->vel = vec2f_div(obj->vel, fixedpt_xdiv(speed, speed_cap));
        }

        // Make this object orbit around the center of the arena
        /*obj->vel.x += 0.01f;*/
        /*obj->vel.y += 0.01f;*/
        obj->pos.fx += obj->vel.fx;
        obj->pos.fy += obj->vel.fy;
        // obj->pos.x = obj->orbit_pos.x+obj->orbit_pos_vary.x;
        // obj->pos.y = obj->orbit_pos.y+obj->orbit_pos_vary.y;
        // obj->orbit_pos.x += 2*obj->orbit_dest_dir.x;
        // obj->orbit_pos.y += 2*obj->orbit_dest_dir.y;
        // obj->orbit_pos_vary.x += sin(obj->orbit_tick)*0.2f;
        // obj->orbit_pos_vary.y += cos(obj->orbit_tick)*0.6f;
    }
}

int hazard_create(object *obj, scene *scene) {

    object_set_spawn_cb(obj, hazard_spawn_cb, NULL);
    object_set_destroy_cb(obj, cb_scene_destroy_object, NULL);
    object_set_move_cb(obj, hazard_move);
    object_set_dynamic_tick_cb(obj, hazard_tick);

    obj->orbit_pos.fx = obj->pos.fx;
    obj->orbit_pos.fy = obj->pos.fy;
    obj->orbit_dest = vec2f_create(random_fixedpt(&obj->gs->rand, fixedpt_fromint(280)) + fixedpt_fromint(20),
                                   random_fixedpt(&obj->gs->rand, fixedpt_fromint(160)) + fixedpt_fromint(20));
    log_debug("new position is %f, %f", fixedpt_tofloat(obj->orbit_dest.fx), fixedpt_tofloat(obj->orbit_dest.fy));

    return 0;
}
