#include <stdlib.h>
#include "game/utils/har_screencap.h"
#include "utils/log.h"

void har_screencaps_create(har_screencaps *caps) {
    caps->max_damage_value = 0;
    caps->max_damage_screen = NULL;
    caps->last_strike_screen = NULL;
}

void har_screencaps_free(har_screencaps *caps) {
    if(caps->max_damage_screen) {
        surface_free(caps->max_damage_screen);
        caps->max_damage_screen = NULL;
    }
    if(caps->last_strike_screen) {
        surface_free(caps->last_strike_screen);
        caps->last_strike_screen = NULL;
    }
}

void har_screencaps_reset(har_screencaps *caps) {
    har_screencaps_free(caps);
    caps->max_damage_value = 0;
}

void har_screencaps_capture_dmg(har_screencaps *caps, object *obj, float dmg) {
    if(dmg > caps->max_damage_value) {
        caps->max_damage_value = dmg;

        // TODO: Calculate correct capture position from obj. position
        // TODO: Capture screen
        // TODO: Crop

        DEBUG("Screencapture at dmg = %f", dmg);
    }
}

void har_screencaps_capture_last(har_screencaps *caps, object *obj) {
    // TODO: Calculate correct capture position from obj. position
    // TODO: Capture screen
    // TODO: Crop

    DEBUG("Screencapture at arena end");
}