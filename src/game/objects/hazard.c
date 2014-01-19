#include <stdlib.h>
#include "game/objects/hazard.h"
#include "game/protos/object_specializer.h"
#include "utils/log.h"
#include "game/protos/scene.h"


void hazard_spawn_cb(object *parent, int id, vec2i pos, int g, void *userdata) {
    scene *s = (scene*)userdata;

    // Get next animation
    bk_info *info = bk_get_info(&s->bk_data, id);
    if(info != NULL) {
        object *obj = malloc(sizeof(object));
        object_create(obj, parent->gs, vec2i_add(pos, info->ani.start_pos), vec2f_create(0,0));
        object_set_stl(obj, object_get_stl(parent));
        object_set_animation(obj, &info->ani);
        if(info->probability == 1) {
            object_set_repeat(obj, 1);
        }
        DEBUG("hazard spawned child");
        object_set_layers(obj, LAYER_HAZARD|LAYER_HAR);
        object_set_group(obj, GROUP_PROJECTILE);
        object_set_userdata(obj, object_get_userdata(parent));
        hazard_create(obj, s);
        if (s->bk_data.file_id == 128 && id == 14) {
            // XXX hack because we don't understand the ms and md tags
            // without this, the 'bullet damage' sprite in the desert spawns at 0,0
            obj->pos = parent->pos;
        }
        game_state_add_object(parent->gs, obj, RENDER_LAYER_BOTTOM);
    } else {
        DEBUG("failed to spawn hazard child");
    }
}

int hazard_create(object *obj, scene *scene) {

    object_set_spawn_cb(obj, hazard_spawn_cb, (void*)scene);
    object_set_destroy_cb(obj, cb_scene_destroy_object, (void*)scene);

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
    DEBUG("unserializing hazard");
    bk *bk_data = &gs->sc->bk_data;
    hazard_create(obj, gs->sc);
    object_set_userdata(obj, bk_data);
    object_set_stl(obj, bk_data->sound_translation_table);
    object_set_animation(obj, &bk_get_info(bk_data, animation_id)->ani);
    return 0;
}

void hazard_bootstrap(object *obj) {
    DEBUG("bootstrapping hazard");
    object_set_serialize_cb(obj, hazard_serialize);
    object_set_unserialize_cb(obj, hazard_unserialize);
}


