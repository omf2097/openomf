/**
 * @file dat_source.h
 * @brief SOUNDS.DAT sound_source loader
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef DAT_SOUND_SOURCE_H
#define DAT_SOUND_SOURCE_H

#include "audio/sound_sources/sound_source.h"

#include <stdbool.h>

/**
 * @brief Load a sample from the SOUNDS.DAT.
 * @param src Sound source to populate.
 * @param sound_id SOUNDS.DAT sample id.
 * @return true on success, false if sound_id is out of range or the slot is empty.
 */
bool dat_source_load(sound_source *src, int sound_id);

#endif // DAT_SOUND_SOURCE_H
