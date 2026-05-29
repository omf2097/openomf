/**
 * @file playing_sound.h
 * @brief In-flight sound record (private header, but used by tests)
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef PLAYING_SOUND_H
#define PLAYING_SOUND_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief In-flight sound record used for rollback replay.
 * @details Stores the data needed to fade-resume a sound at an offset after a rollback
 *          discards the playback in progress.
 */
typedef struct {
    int tick;                  ///< Game tick at which the sound started. Used as merge identity.
    int sound_id;              ///< Sample id. Used as merge identity together with `tick`.
    int length;                ///< Original sample length in bytes.
    int duration;              ///< Milliseconds of playback remaining; expires when <= 0.
    int total_duration_ms;     ///< Initial duration in ms. Used for pan-sweep.
    int volume;                ///< Per-play volume, 0 ... 127.
    int panning;               ///< Current panning, -100 ... 100.
    int pan_start;             ///< Start panning.
    int pan_end;               ///< Target panning.
    bool has_pan_sweep;        ///< True if pan-sweep is enabled.
    uint32_t follow_object_id; ///< Object ID to track (0 = no tracking)
    int pitch;                 ///< Per-play pitch adjustment (see pitched_samplerate).
    int playback_id;           ///< Backend channel id, or -1 if not yet handed to the backend.
} playing_sound;

#endif // PLAYING_SOUND_H
