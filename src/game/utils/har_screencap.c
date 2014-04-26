#include <stdlib.h>
#include <stdio.h>
#include "game/utils/har_screencap.h"
#include "utils/log.h"
#include "video/video.h"
#include "video/image.h"

void har_screencaps_create(har_screencaps *caps) {
    caps->max_damage_value = 0;
    caps->max_damage_ok = 0;
    caps->last_strike_ok = 0;
}

void har_screencaps_free(har_screencaps *caps) {
    if(caps->max_damage_ok) {
        surface_free(&caps->max_damage_screen);
        caps->max_damage_ok = 0;
    }
    if(caps->last_strike_ok) {
        surface_free(&caps->last_strike_screen);
        caps->last_strike_ok = 0;
    }
}

void har_screencaps_reset(har_screencaps *caps) {
    har_screencaps_free(caps);
    caps->max_damage_value = 0;
}

int _capture(surface *sur, object *obj) {
    // Find screencap position
    int mx = 0;
    if(object_get_direction(obj) == OBJECT_FACE_LEFT)
        mx -= 20;
    else
        mx += 20;

    int x = (object_px(obj)+mx) - (SCREENCAP_W - object_w(obj)) / 2;
    int y = (object_py(obj)-object_h(obj)) - (SCREENCAP_H - object_h(obj)) / 2;
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x + SCREENCAP_W >= NATIVE_W) x = NATIVE_W - SCREENCAP_W;
    if(y + SCREENCAP_H >= NATIVE_H) y = NATIVE_H - SCREENCAP_H;

    // Capture
    return video_area_capture(sur, x, y, SCREENCAP_W, SCREENCAP_H);
}

void har_screencaps_capture_dmg(har_screencaps *caps, object *obj, float dmg) {
    if(dmg > caps->max_damage_value) {
        caps->max_damage_value = dmg;
        if(caps->max_damage_ok) {
            surface_free(&caps->max_damage_screen);
            caps->max_damage_ok = 0;
        }
        if(_capture(&caps->max_damage_screen, obj) == 0) {
            caps->max_damage_ok = 1;
        }
    }
}

void har_screencaps_capture_last(har_screencaps *caps, object *obj) {
    if(caps->last_strike_ok) {
        surface_free(&caps->last_strike_screen);
        caps->last_strike_ok = 0;
    }
    if(_capture(&caps->last_strike_screen, obj) == 0) {
        caps->last_strike_ok = 1;
    }
}