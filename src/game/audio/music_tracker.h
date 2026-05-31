/**
 * @file music_tracker.h
 * @brief Music playback tracker and hnadler.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef MUSIC_TRACKER_H
#define MUSIC_TRACKER_H

#include "resources/ids.h"

/**
 * @brief Start playback of a song if it's not already playing.
 * @param id Music resource id.
 */
void music_tracker_play(resource_id id);

/**
 * @brief Stop the currently playing song.
 */
void music_tracker_stop(void);

#endif // MUSIC_TRACKER_H
