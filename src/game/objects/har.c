#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "game/har.h"
#include "resources/af_loader.h"
#include "resources/ids.h"
#include "resources/animation.h"

void har_free(object *obj) {
    har *h = object_get_userdata(obj);
    af_free(&h->af_data);
}

void har_tick(har *har) {
    //har *h = object_get_userdata(obj);
}

void har_act(har *har, int act_type) {
    //har *h = object_get_userdata(obj);
}

int har_create(object *obj, palette *pal, int dir, int har_id) {
    // Create local data
    har *local = malloc(sizeof(har));

    // Load AF
    if(load_af_file(&local->af_data, har_id)) {
        PERROR("Unable to load HAR %s (%d)!", get_id_name(har_id), har_id);
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

    // Set running animation (initialize with palette before setting)
    animation *idle_animation = af_get_move(&local->af_data, ANIM_IDLE).animation;
    animation_init(idle_animation, palette, 0);
    object_set_animation(obj, idle_animation);

    // Callbacks and userdata
    object_set_userdata(obj, local);
    object_set_free_cb(obj, har_free);
    object_set_act_cb(object, har_act);
    object_set_tick_cb(object, har_tick);

    // All done
    DEBUG("Har %s (%d) loaded!", get_id_name(har_id), har_id);
    return 0;
}