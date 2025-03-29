#include "video/damage_tracker.h"
#include "utils/miscmath.h"
#include <string.h>

void damage_reset(damage_tracker *tracker) {
    tracker->dirty = false;
    tracker->dirty_range_last = 0;
    tracker->dirty_range_first = 255;
}

void damage_combine(damage_tracker *dst, const damage_tracker *src) {
    if(!src->dirty) {
        return;
    }
    dst->dirty = true;
    dst->dirty_range_first = min2(dst->dirty_range_first, src->dirty_range_first);
    dst->dirty_range_last = max2(dst->dirty_range_last, src->dirty_range_last);
}

void damage_add_range(damage_tracker *tracker, vga_index start, vga_index end) {
    vga_index last = end + (vga_index)255;
    assert(last >= start);
    tracker->dirty = true;
    tracker->dirty_range_first = min2(tracker->dirty_range_first, start);
    tracker->dirty_range_last = max2(tracker->dirty_range_last, last);
}

void damage_set_all(damage_tracker *tracker) {
    tracker->dirty = true;
    tracker->dirty_range_first = 0;
    tracker->dirty_range_last = 255;
}
