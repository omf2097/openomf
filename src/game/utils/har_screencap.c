#include "game/utils/har_screencap.h"
#include "utils/miscmath.h"
#include "video/vga_state.h"
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

static vec2i camera_position_for(object *obj) {
    vec2i size = object_get_size(obj);
    vec2i pos = object_get_pos(obj);
    int x_margin = (SCREENCAP_W - size.x) / 2;
    int y_margin = (SCREENCAP_H - size.y) / 2;
    int x_center = pos.x - size.x / 2;
    int y_center = (NATIVE_H - pos.y);

    vec2i res;
    res.x = clamp(x_center - x_margin, 0, NATIVE_W - SCREENCAP_W);
    res.y = clamp(y_center + y_margin, 0, NATIVE_H);
    return res;
}

void har_screencaps_capture(har_screencaps *caps, object *obj, object *obj2, int id) {
    game_state *gs = obj->gs;
    if(caps->ok[id]) {
        surface_free(&caps->cap[id]);
        caps->ok[id] = false;
    }

    // Position
    vec2i pos = camera_position_for(obj);
    vec2i size = {SCREENCAP_W, SCREENCAP_H};
    if(obj2) {
        // pan out when bots are apart
        vec2i pos2 = camera_position_for(obj2);
        size.x = abs(pos.x - pos2.x) + size.x;
        size.y = SCREENCAP_H * size.x / SCREENCAP_W;
        pos.x = min2(pos.x, pos2.x);
        pos.y -= (size.y - SCREENCAP_H) / 2;
    }

    // Render offscreen and capture.
    SDL_Rect clip_area = {pos.x, pos.y, size.x, size.y};
    video_render_area_prepare(&clip_area);
    gs->hide_ui = true;
    game_state_render(gs);
    gs->hide_ui = false;
    video_render_area_finish(&caps->cap[id]);
    caps->ok[id] = true;
}

void har_screencaps_compress(har_screencaps *caps, const vga_palette *pal, int id) {
    surface_convert_to_grayscale(&caps->cap[id], pal, 0xD0, 0xDF, 0x60);
}
