#include "game/objects/hazard.h"
#include "game/protos/scene.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include <math.h>
#include <stdlib.h>

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

int hazard_create(object *obj, scene *scene) {
    object_set_spawn_cb(obj, hazard_spawn_cb, NULL);
    object_set_destroy_cb(obj, cb_scene_destroy_object, NULL);
    object_set_dynamic_tick_cb(obj, hazard_tick);

    return 0;
}
