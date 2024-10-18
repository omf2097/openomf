#include "video/damage_tracker.h"
#include "utils/miscmath.h"
#include <string.h>

void damage_reset(damage_tracker *tracker) {
    tracker->dirty = false;
    tracker->dirty_range_end = 0;
    tracker->dirty_range_start = 255;
}

void damage_copy(damage_tracker *dst, const damage_tracker *src) {
    memcpy(dst, src, sizeof(damage_tracker));
}

void damage_set_range(damage_tracker *tracker, vga_index start, vga_index end) {
    tracker->dirty = true;
    tracker->dirty_range_start = min2(tracker->dirty_range_start, start);
    tracker->dirty_range_end = max2(tracker->dirty_range_end, end);
}
