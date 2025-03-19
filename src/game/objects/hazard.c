#include "game/objects/hazard.h"
#include "game/protos/scene.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include <math.h>
#include <stdlib.h>

int orb_almost_there(vec2f a, vec2f b) {
    vec2f dir = vec2f_sub(a, b);
    return (dir.x >= -2.0f && dir.x <= 2.0f && dir.y >= -2.0f && dir.y <= 2.0f);
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
            obj->orbit_pos = vec2i_to_f(object_get_pos(obj));
            obj->orbit_pos_vary = vec2f_create(0, 0);
            float mag;
            int limit = 10;
            do {
                obj->orbit_dest =
                    vec2f_create(random_float(&obj->gs->rand) * 320.0f, random_float(&obj->gs->rand) * 200.0f);
                obj->orbit_dest_dir = vec2f_sub(obj->orbit_dest, obj->orbit_pos);
                mag = sqrtf(obj->orbit_dest_dir.x * obj->orbit_dest_dir.x +
                            obj->orbit_dest_dir.y * obj->orbit_dest_dir.y);
                limit--;
            } while(mag < 80.0f && limit > 0);

            obj->orbit_dest_dir.x /= mag;
            obj->orbit_dest_dir.y /= mag;
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
    vec2f new =
        vec2f_create((random_float(&obj->gs->rand) * 280.0f) + 20.0f, (random_float(&obj->gs->rand) * 160.0f) + 20.0f);
    while(vec2f_dist(old, new) < 100) {
        new = vec2f_create((random_float(&obj->gs->rand) * 280.0f) + 20.0f,
                           (random_float(&obj->gs->rand) * 160.0f) + 20.0f);
    }
    return new;
}

void accelerate_orbit(object *obj) {
    float x_dist = object_px(obj) - obj->orbit_dest.x;
    float y_dist = object_py(obj) - obj->orbit_dest.y;
    float bigger = max2(x_dist, y_dist);
    if(fabsf(bigger) > 20) {
        bigger *= -1.0f;
    }
    if(obj->vel.x < 1.0f) {
        obj->vel.x += x_dist / (bigger * 10);
    }
    if(obj->vel.y < 1.0f) {
        obj->vel.y += y_dist / (bigger * 10);
    }
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

        if((dist(object_px(obj), obj->orbit_pos.x) >= dist(obj->orbit_dest.x, obj->orbit_pos.x)) &&
           (dist(object_py(obj), obj->orbit_pos.y) >= dist(obj->orbit_dest.y, obj->orbit_pos.y))) {
            obj->orbit_pos.x = object_px(obj);
            obj->orbit_pos.y = object_py(obj);
            obj->orbit_dest = generate_destination(obj);
            log_debug("new position is %f, %f", obj->orbit_dest.x, obj->orbit_dest.y);
        }

        // accelerate_orbit(obj);

        float x_dist = object_px(obj) - obj->orbit_dest.x;
        float y_dist = object_py(obj) - obj->orbit_dest.y;
        float bigger = fabsf(y_dist);
        if(fabsf(x_dist) > fabsf(y_dist)) {
            bigger = x_dist;
        }

        if(obj->orbit_dest.x > object_px(obj)) {
            if(obj->vel.x < 1.0f) {
                log_debug("accel +%f", x_dist / (bigger * 10));
                obj->vel.x += x_dist / (bigger * 10);
            }
        }
        if(obj->orbit_dest.x < object_px(obj)) {
            if(obj->vel.x < 1.0f) {
                log_debug("accel -%f", x_dist / (bigger * 10));
                obj->vel.x -= x_dist / (bigger * 10);
            }
        }
        if(obj->orbit_dest.y > object_py(obj)) {
            if(obj->vel.y < 1.0f) {
                log_debug("accel +%f", y_dist / (bigger * 10));
                obj->vel.y += y_dist / (bigger * 10);
            }
        }
        if(obj->orbit_dest.y < object_py(obj)) {
            if(obj->vel.y < 1.0f) {
                log_debug("accel -%f", y_dist / (bigger * 10));
                obj->vel.y -= y_dist / (bigger * 10);
            }
        }

        // Make this object orbit around the center of the arena
        /*obj->vel.x += 0.01f;*/
        /*obj->vel.y += 0.01f;*/
        obj->pos.x += obj->vel.x * 256;
        obj->pos.y += obj->vel.y * 256;
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

    obj->orbit_pos.x = object_px(obj);
    obj->orbit_pos.y = object_py(obj);
    obj->orbit_dest =
        vec2f_create((random_float(&obj->gs->rand) * 280.0f) + 20.0f, (random_float(&obj->gs->rand) * 160.0f) + 20.0f);
    log_debug("new position is %f, %f", obj->orbit_dest.x, obj->orbit_dest.y);

    return 0;
}
