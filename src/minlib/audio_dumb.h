#ifndef AUDIO_DUMB_H
#define AUDIO_DUMB_H

#include "audio/backends/audio_backend.h"

/**
 * @brief Populate `backend` with the waveOut backend's callbacks.
 * @param backend Backend vtable to fill.
 */
void dumb_audio_backend_set_callbacks(audio_backend *backend);

#endif /* AUDIO_DUMB_H */
