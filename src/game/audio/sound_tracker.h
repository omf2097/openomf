/**
 * @file sound_tracker.h
 * @brief Rollback-aware sound playback tracker
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SOUND_TRACKER_H
#define SOUND_TRACKER_H

#include "audio/sound_opts.h"
#include "utils/vector.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Callback for looking up the current pan of a tracked object.
 * @param ctx Freeform context object
 * @param object_id Object which is queried
 * @return Pan value in -100..100, or INT_MIN if the object disappeared
 */
typedef int (*sound_pan_lookup)(void *ctx, uint32_t object_id);

/**
 * @brief Per-game-state tracker. Cloned alongside game_state for rollback.
 */
typedef struct sound_tracker {
    vector entries; ///< In-flight sound entries.
} sound_tracker;

/**
 * @brief Initialize a tracker with an empty entry vector.
 * @param t Tracker to initialize.
 */
void sound_tracker_create(sound_tracker *t);

/**
 * @brief Release the tracker's entry vector.
 * @param t Tracker to free.
 */
void sound_tracker_free(sound_tracker *t);

/**
 * @brief Clone the tracker instance
 * @param dst Empty destination tracker.
 * @param src Source tracker to copy from.
 */
void sound_tracker_clone(sound_tracker *dst, const sound_tracker *src);

/**
 * @brief Advance the tracker by one tick: decay durations, drop expired entries, and
 *        recompute per-tick panning for entries with sweep or follow set.
 * @param t Tracker to advance.
 * @param ms_elapsed Milliseconds since the last tick.
 * @param lookup Object lookup callback for follow-source pan. May be NULL.
 * @param ctx Context passed through to the lookup.
 */
void sound_tracker_tick(sound_tracker *t, int ms_elapsed, sound_pan_lookup lookup, void *ctx);

/**
 * @brief Reconcile playback state across a rollback.
 * @param old Tracker before rollback.
 * @param new Tracker after rollback.
 */
void sound_tracker_merge(sound_tracker *old, sound_tracker *new);

/**
 * @brief Play a sound and record it for rollback.
 * @param t Tracker to record into.
 * @param tick The current game tick (stored on the entry for merge matching).
 * @param clone True if this is being called on a cloned/lookahead state
 * @param sound_id Sample id (range-checked, ignored if outside 0 ... 299).
 * @param opts Per-play parameters, or NULL for defaults.
 */
void sound_tracker_play(sound_tracker *t, int tick, bool clone, int sound_id, const sound_opts *opts);

#endif // SOUND_TRACKER_H
