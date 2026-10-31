/**
 * @file sdl_backend.h
 * @brief SDL_mixer audio backend
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SDL_BACKEND_H
#define SDL_BACKEND_H

#include "audio/backends/audio_backend.h"

void sdl_audio_backend_set_callbacks(audio_backend *sdl_player);

#endif // SDL_BACKEND_H
