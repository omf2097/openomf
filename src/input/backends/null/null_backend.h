/**
 * @file null_backend.h
 * @brief No-op input backend for headless / test builds
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef NULL_INPUT_BACKEND_H
#define NULL_INPUT_BACKEND_H

#include "input/backends/input_backend.h"

/**
 * @brief Populate `null_backend` with the no-op backend's callbacks.
 * @param null_backend Backend vtable to fill.
 */
void null_input_backend_set_callbacks(input_backend *null_backend);

#endif // NULL_INPUT_BACKEND_H
