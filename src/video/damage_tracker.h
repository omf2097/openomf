#ifndef DAMAGE_TRACKER_H
#define DAMAGE_TRACKER_H

#include "video/vga_palette.h"
#include <stdbool.h>

/**
 * \brief Tracks what vga_indexes are dirty, if any are.
 */
typedef struct damage_tracker {
    vga_index dirty_range_first;
    vga_index dirty_range_last;
    bool dirty;
} damage_tracker;

/**
 * \brief Clears all damage from a tracker.
 *
 * \param tracker
 */
void damage_reset(damage_tracker *tracker);

/**
 * \brief Overwrites a tracker with another entirely.
 *
 * \param dst
 * \param src
 */
inline static void damage_copy(damage_tracker *dst, const damage_tracker *src) {
    *dst = *src;
}

/**
 * \brief Combines the damage from src into dst, so that
 * everything that is damaged in src is now also damaged in
 * dst.
 *
 * \param dst
 * \param src
 */
void damage_combine(damage_tracker *dst, const damage_tracker *src);

/**
 * \brief Marks a non-empty contiguous range as damaged.
 *
 * \param tracker
 * \param start the first index of the range to dirty
 * \param end the end of the range, will not be dirtied.
 */
void damage_add_range(damage_tracker *tracker, vga_index start, vga_index end);

/**
 * \brief Marks entire tracker as damaged.
 *
 * \param tracker
 */
void damage_set_all(damage_tracker *tracker);

#endif // DAMAGE_TRACKER_H
