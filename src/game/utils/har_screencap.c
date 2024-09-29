#include "game/utils/har_screencap.h"
#include "utils/miscmath.h"
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
            surface_create_from(&dst->cap[i], &src->cap[i]);
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
    vec2i size = object_get_size(obj);
    vec2i pos = object_get_pos(obj);
    int x_margin = (SCREENCAP_W - size.x) / 2;
    int y_margin = (SCREENCAP_H - size.y) / 2;
    int x_center = pos.x - size.x / 2;
    int y_center = (NATIVE_H - pos.y);
    int x = clamp(x_center - x_margin, 0, NATIVE_W - SCREENCAP_W);
    int y = clamp(y_center + y_margin, 0, NATIVE_H);

    // Capture
    video_area_capture(&caps->cap[id], x, y, SCREENCAP_W, SCREENCAP_H);
    surface_convert_to_grayscale(&caps->cap[id], video_get_pal_ref(), 0xD0, 0xDF);
    caps->ok[id] = true;
}
