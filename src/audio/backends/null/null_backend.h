/**
 * @file null_backend.h
 * @brief No-op audio backend for headless / test builds
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef NULL_BACKEND_H
#define NULL_BACKEND_H

#include "audio/backends/audio_backend.h"

void null_audio_backend_set_callbacks(audio_backend *null_backend);

#endif // NULL_BACKEND_H
