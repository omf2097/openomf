#include "game/objects/hazard.h"
#include "game/protos/object_specializer.h"
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
    bk *bk_data = (bk *)object_get_userdata(obj);

    if (obj->animation_state.finished) {
        bk_info *anim = bk_get_info(bk_data, obj->cur_animation->id);
        if (anim->chain_no_hit) {
            object_set_animation(obj, &bk_get_info(bk_data, anim->chain_no_hit)->ani);
            object_set_repeat(obj, 0);
            obj->animation_state.finished = 0;
        }
    }
    if (obj->orbit) {
        obj->orbit_tick += MATH_PI / 32;
        if (obj->orbit_tick >= MATH_PI * 2) {
            obj->orbit_tick -= MATH_PI * 2;
        }
        if (orb_almost_there(obj->orbit_dest, obj->orbit_pos)) {
            // XXX come up with a better equation to randomize the destination
            obj->orbit_pos = obj->pos;
            obj->orbit_pos_vary = vec2f_create(0, 0);
            float mag;
            int limit = 10;
            do {
                obj->orbit_dest = vec2f_create(rand_float() * 320.0f, rand_float() * 200.0f);
                obj->orbit_dest_dir = vec2f_sub(obj->orbit_dest, obj->orbit_pos);
                mag = sqrtf(obj->orbit_dest_dir.x * obj->orbit_dest_dir.x +
                            obj->orbit_dest_dir.y * obj->orbit_dest_dir.y);
                limit--;
            } while (mag < 80.0f && limit > 0);

            obj->orbit_dest_dir.x /= mag;
            obj->orbit_dest_dir.y /= mag;
        }
    }
}

void hazard_spawn_cb(object *parent, int id, vec2i pos, vec2f vel, uint8_t flags, int s, int g,
                     void *userdata) {
    scene *sc = (scene *)userdata;

    // Get next animation
    bk_info *info = bk_get_info(&sc->bk_data, id);
    if (info != NULL) {
        object *obj = omf_calloc(1, sizeof(object));
        object_create(obj, parent->gs, vec2i_add(pos, info->ani.start_pos), vel);
        object_set_stl(obj, object_get_stl(parent));
        object_set_animation(obj, &info->ani);
        if (info->probability == 1) {
            object_set_repeat(obj, 1);
        }
        object_set_layers(obj, LAYER_HAZARD | LAYER_HAR);
        object_set_group(obj, GROUP_PROJECTILE);
        object_set_userdata(obj, object_get_userdata(parent));
        hazard_create(obj, sc);
        if (s) {
            // If MS tag is set, correct the bullet damage animation position
            obj->pos = parent->pos;
        }
        game_state_add_object(parent->gs, obj, RENDER_LAYER_BOTTOM, 0, 0);
    } else {
        DEBUG("failed to spawn hazard child");
    }
}

void hazard_move(object *obj) {
    if (obj->orbit) {
        // Make this object orbit around the center of the arena
        obj->pos.x = obj->orbit_pos.x + obj->orbit_pos_vary.x;
        obj->pos.y = obj->orbit_pos.y + obj->orbit_pos_vary.y;
        obj->orbit_pos.x += 2 * obj->orbit_dest_dir.x;
        obj->orbit_pos.y += 2 * obj->orbit_dest_dir.y;
        obj->orbit_pos_vary.x += sin(obj->orbit_tick) * 0.2f;
        obj->orbit_pos_vary.y += cos(obj->orbit_tick) * 0.6f;
    }
}

int hazard_create(object *obj, scene *scene) {

    object_set_spawn_cb(obj, hazard_spawn_cb, (void *)scene);
    object_set_destroy_cb(obj, cb_scene_destroy_object, (void *)scene);
    object_set_move_cb(obj, hazard_move);
    object_set_dynamic_tick_cb(obj, hazard_tick);

    hazard_bootstrap(obj);

    return 0;
}

int hazard_serialize(object *obj, serial *ser) {
    /*DEBUG("serializing hazard");*/
    // Specialization
    serial_write_int8(ser, SPECID_HAZARD);
    return 0;
}

int hazard_unserialize(object *obj, serial *ser, int animation_id, game_state *gs) {
    bk *bk_data = &gs->sc->bk_data;
    hazard_create(obj, gs->sc);
    object_set_userdata(obj, bk_data);
    object_set_stl(obj, bk_data->sound_translation_table);
    object_set_animation(obj, &bk_get_info(bk_data, animation_id)->ani);
    return 0;
}

void hazard_bootstrap(object *obj) {
    object_set_serialize_cb(obj, hazard_serialize);
    object_set_unserialize_cb(obj, hazard_unserialize);
}
