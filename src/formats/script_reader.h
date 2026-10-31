/**
 * @file script_reader.h
 * @brief Stateful playback cursor over a decoded animation script.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SCRIPT_READER_H
#define SCRIPT_READER_H

#include "formats/script.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct script_reader {
    // State
    const script *script;   ///< Borrowed, read-only script reference.
    uint32_t tick;          ///< Current playback position in ticks.
    uint32_t previous_tick; ///< Position before the last move, for frame-change detection.

    // Cache
    const script_frame *frame; ///< Cached current frame, or NULL when out of range.
    int frame_index;           ///< Index of the cached frame.
    uint32_t frame_start;      ///< First tick of the cached frame.
    uint32_t frame_end;        ///< One past the last tick of the cached frame.
    bool frame_valid;          ///< Whether the cached frame window is valid.
} script_reader;

/** @brief Point the reader at a script (resets playback and the frame cache). */
void script_reader_load(script_reader *r, const script *script);

/** @brief Reset the playback state: position to the start and previous position cleared. */
void script_reader_reset(script_reader *r);

/** @brief Set the playback position to an absolute tick. */
void script_reader_seek(script_reader *r, uint32_t tick);

/** @brief Move the playback position by a relative amount (usually +1 per game tick). */
void script_reader_advance(script_reader *r, int n);

/** @brief Returns the current playback position in ticks. */
uint32_t script_reader_tick(const script_reader *r);

/** @brief Remember the current position as the previous one. */
void script_reader_mark_previous(script_reader *r);

/** @brief Mark the current frame as freshly entered, so the next frame-change check reports a change. */
void script_reader_mark_entered(script_reader *r);

/** @brief Read-only access to the script instance. */
const script *script_reader_get_script(const script_reader *r);

/** @brief Returns the frame at the current position (cached), or NULL if out of range. */
const script_frame *script_reader_frame(const script_reader *r);

/** @brief Tells if the frame changed between the previous and the current position. */
bool script_reader_frame_changed(const script_reader *r);

/** @brief Tells if the tag is set in the frame at the current position. */
bool script_reader_isset(const script_reader *r, script_tag tag);

/** @brief Returns the value of the tag in the frame at the current position, or 0. */
int script_reader_get(const script_reader *r, script_tag tag);

#endif // SCRIPT_READER_H
