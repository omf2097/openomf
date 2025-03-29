#ifndef DAMAGE_TRACKER_H
#define DAMAGE_TRACKER_H

#include "video/vga_palette.h"
#include <stdbool.h>

typedef struct damage_tracker {
    vga_index dirty_range_first;
    vga_index dirty_range_last;
    bool dirty;
} damage_tracker;

void damage_reset(damage_tracker *tracker);
void damage_copy(damage_tracker *dst, const damage_tracker *src);
void damage_combine(damage_tracker *dst, const damage_tracker *src);

// Marks a contigous range as damaged.
//
// The range starts at index `start`, and does not include index `end`.
// A common usage pattern is `damage_add_range(tracker, start, start + count);`
void damage_add_range(damage_tracker *tracker, vga_index start, vga_index end);
// Marks everything as damaged.
void damage_set_all(damage_tracker *tracker);

#endif // DAMAGE_TRACKER_H
