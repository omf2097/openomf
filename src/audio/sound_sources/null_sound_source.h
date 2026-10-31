/**
 * @file null_sound_source.h
 * @brief NULL sound_source for tests
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef NULL_SOUND_SOURCE_H
#define NULL_SOUND_SOURCE_H

#include "audio/sound_sources/sound_source.h"

#include <stdbool.h>

bool null_sound_source_load(sound_source *src, int sound_id);

#endif // NULL_SOUND_SOURCE_H
