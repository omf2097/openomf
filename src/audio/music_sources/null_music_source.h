/**
 * @file null_music_source.h
 * @brief NULL music source for tests
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef NULL_MUSIC_SOURCE_H
#define NULL_MUSIC_SOURCE_H

#include "audio/music_sources/music_source.h"

#include <stdbool.h>

bool null_music_source_load(music_source *src);

#endif // NULL_MUSIC_SOURCE_H
