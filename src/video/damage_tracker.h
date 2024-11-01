#ifndef DAMAGE_TRACKER_H
#define DAMAGE_TRACKER_H

#include "video/vga_palette.h"
#include <stdbool.h>

typedef struct damage_tracker {
    vga_index dirty_range_start;
    vga_index dirty_range_end;
    bool dirty;
} damage_tracker;

void damage_reset(damage_tracker *tracker);
void damage_copy(damage_tracker *dst, const damage_tracker *src);
void damage_set_range(damage_tracker *tracker, vga_index start, vga_index end);

#endif // DAMAGE_TRACKER_H
