#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "game/objects/har.h"
#include "resources/af_loader.h"
#include "resources/ids.h"
#include "resources/animation.h"
#include "utils/log.h"

void har_free(object *obj) {
    har *h = object_get_userdata(obj);
    af_free(&h->af_data);
}

void har_tick(object *har) {
    //har *h = object_get_userdata(obj);
}

void har_act(object *har, int act_type) {
    //har *h = object_get_userdata(obj);
}

int har_create(object *obj, palette *pal, int dir, int har_id) {
    // Create local data
    har *local = malloc(sizeof(har));

    // Load AF
    if(load_af_file(&local->af_data, har_id)) {
        PERROR("Unable to load HAR %s (%d)!", get_id_name(har_id), har_id);
        free(local);
        return 1;
    }

    // Health, endurance
    local->health_max = local->health = 1000;
    local->endurance_max = local->endurance = 1000;

    // Object related stuff
    object_set_gravity(obj, 1.0f);
    object_set_layers(obj, LAYER_HAR);
    object_set_direction(obj, dir);
    object_set_repeat(obj, 1);

    // Set running animation 
    object_set_animation(obj, &af_get_move(&local->af_data, ANIM_IDLE)->ani);
    object_set_palette(obj, pal, 0);

    // Callbacks and userdata
    object_set_userdata(obj, local);
    object_set_free_cb(obj, har_free);
    object_set_act_cb(obj, har_act);
    object_set_tick_cb(obj, har_tick);

    // All done
    DEBUG("Har %s (%d) loaded!", get_id_name(har_id), har_id);
    return 0;
}