#ifndef _HAR_SCREENCAP_H
#define _HAR_SCREENCAP_H

#include "game/protos/object.h"
#include "video/surface.h"

#define SCREENCAP_W 140
#define SCREENCAP_H 100

// There should be screencaps for each HAR/player
typedef struct {
    float max_damage_value;
    surface max_damage_screen;
    int max_damage_ok;
    surface last_strike_screen;
    int last_strike_ok;
} har_screencaps;

void har_screencaps_create(har_screencaps *caps);
void har_screencaps_free(har_screencaps *caps);
void har_screencaps_reset(har_screencaps *caps);

// This should be used every time a HAR damages another HAR
// Screenshot routines should only fire, if damage is higher than last time
// This would be the second picture shown in the newsroom
void har_screencaps_capture_dmg(har_screencaps *caps, object *obj, float dmg);

// This should be used on the last attack only
// This would be the first picture shown in newsroom
void har_screencaps_capture_last(har_screencaps *caps, object *obj);

#endif // _HAR_SCREENCAP_H