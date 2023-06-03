#include "game/utils/har_screencap.h"
#include "video/video.h"

void har_screencaps_create(har_screencaps *caps) {
    for(int i = 0; i < 2; i++) {
        caps->ok[i] = false;
    }
}

void har_screencaps_free(har_screencaps *caps) {
    for(int i = 0; i < 2; i++) {
        if(caps->ok[i]) {
            surface_free(&caps->cap[i]);
            caps->ok[i] = false;
        }
    }
}

void har_screencaps_reset(har_screencaps *caps) {
    har_screencaps_free(caps);
}

int har_screencaps_clone(har_screencaps *src, har_screencaps *dst) {
    for(int i = 0; i < 2; i++) {
        if(src->ok[i]) {
            surface_copy(&dst->cap[i], &src->cap[i]);
        }
    }
    return 1;
}

void har_screencaps_capture(har_screencaps *caps, object *obj, int id) {
    if(caps->ok[id]) {
        surface_free(&caps->cap[id]);
        caps->ok[id] = false;
    }

    // Position
    int x = (object_px(obj) - object_w(obj) / 2) - (SCREENCAP_W - object_w(obj)) / 2;
    int y = (object_py(obj) - object_h(obj)) - (SCREENCAP_H - object_h(obj)) / 2;
    if(x < 0)
        x = 0;
    if(y < 0)
        y = 0;
    if(x + SCREENCAP_W >= NATIVE_W)
        x = NATIVE_W - SCREENCAP_W;
    if(y + SCREENCAP_H >= NATIVE_H)
        y = NATIVE_H - SCREENCAP_H;

    // Capture
    video_area_capture(&caps->cap[id], x, y, SCREENCAP_W, SCREENCAP_H);
    surface_convert_to_grayscale(&caps->cap[id], video_get_pal_ref(), 0xD0, 0xDF);
    caps->ok[id] = true;
}
