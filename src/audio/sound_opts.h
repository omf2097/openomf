/**
 * @file sound_opts.h
 * @brief Per-play sound parameters
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SOUND_OPTS_H
#define SOUND_OPTS_H

/**
 * @brief Per-play sound parameters. NULL at the API means defaults.
 */
typedef struct sound_opts {
    int volume;     ///< 0..127. Matches the original game's MASI driver scale.
    int panning;    ///< -100..100, 0 = center. Matches the `sb` tag scale.
    int pitch;      ///< 0 = native. Matches the `sf` tag scale (audio API applies it).
    int fade_in_ms; ///< 0 = no fade-in.
} sound_opts;

/**
 * @brief Initialize `opts` to sensible defaults (full volume, centered, native pitch, no fade).
 */
static inline void sound_opts_init(sound_opts *opts) {
    opts->volume = 127;
    opts->panning = 0;
    opts->pitch = 0;
    opts->fade_in_ms = 0;
}

#endif // SOUND_OPTS_H
