/**
 * @file sound_opts.h
 * @brief Per-play sound parameters
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SOUND_OPTS_H
#define SOUND_OPTS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Per-play sound parameters. NULL at the API means defaults.
 */
typedef struct sound_opts {
    int volume;                ///< Volume, range is 0 ... 127.
    int panning;               ///< Stereo panning, range is -100 ... 100 (0 = center).
    int panning_end;           ///< Target panning for a sweep
    int pitch;                 ///< Sound pitch, range is -128 ... 128 (0 = no change).
    int fade_in_ms;            ///< Fade-in duration in milliseconds. 0 = no fade-in.
    int priority;              ///< Channel-eviction priority. Higher or equal beats lower or equal.
    int channel;               ///< Force specific channel [0, 1, 2] (-1 = auto).
    bool skip_duplicate;       ///< Skip new sound, if same ID is already playing.
    bool stop_duplicate;       ///< Stop and replace the old sound, if the same ID is already playing.
    bool has_panning_sweep;    ///< True if panning should sweep from `panning` to `panning_end`.
    uint32_t follow_object_id; ///< 0 = no follow; otherwise an object whose pos.x drives panning each tick.
} sound_opts;

/**
 * @brief Initialize opts to sensible defaults.
 * @details Full volume, centered, native pitch, no fade, default priority 10, auto channel.
 * @param opts Options struct to populate.
 */
static inline void sound_opts_init(sound_opts *opts) {
    opts->volume = 127;
    opts->panning = 0;
    opts->panning_end = 0;
    opts->pitch = 0;
    opts->fade_in_ms = 0;
    opts->priority = 10;
    opts->channel = -1;
    opts->skip_duplicate = false;
    opts->stop_duplicate = false;
    opts->has_panning_sweep = false;
    opts->follow_object_id = 0;
}

#endif // SOUND_OPTS_H
