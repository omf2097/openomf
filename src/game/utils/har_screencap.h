#ifndef HAR_SCREENCAP_H
#define HAR_SCREENCAP_H

#include "game/protos/object.h"
#include "video/surface.h"

#define SCREENCAP_W 140
#define SCREENCAP_H 100

#define SCREENCAP_BLOW 0
#define SCREENCAP_POSE 1

// There should be screencaps for each HAR/player
typedef struct har_screencaps {
    surface cap[2];
    bool ok[2];
} har_screencaps;

void har_screencaps_create(har_screencaps *caps);
void har_screencaps_free(har_screencaps *caps);
void har_screencaps_reset(har_screencaps *caps);
int har_screencaps_clone(har_screencaps *src, har_screencaps *dst);
void har_screencaps_capture(har_screencaps *caps, object *obj, object *obj2, int id);
void har_screencaps_compress(har_screencaps *caps, const vga_palette *pal, int id);

#endif // HAR_SCREENCAP_H
