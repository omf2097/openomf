#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

#include "resources/ids.h"

#define VOLUME_DEFAULT 1.0f
#define PANNING_DEFAULT 0.0f
#define PITCH_DEFAULT 1.0f

#define VOLUME_MAX 1.0f
#define PANNING_MAX 1.0f
#define PITCH_MAX 2.0f

#define VOLUME_MIN 0.0f
#define PANNING_MIN -1.0f
#define PITCH_MIN 0.5f

typedef struct audio_mod_resampler {
    int internal_id;
    int is_default;
    const char *name;
} audio_mod_resampler;

typedef struct audio_freq {
    int freq;
    int is_default;
    const char *name;
} audio_freq;

/**
 * Initializes the audio subsystem
 *
 * @param freq Wanted output frequency (48000 should be fine)
 * @param mono True if 1 channel, False for 2.
 * @param resampler Music module resampler interpolation
 * @param music_volume Initial music volume
 * @param sound_volume Initial audio volume
 * @return True if initialized, false if not.
 */
bool audio_init(int freq, bool mono, int resampler, float music_volume, float sound_volume);

/**
 * Closes the audio subsystem.
 */
void audio_close(void);

/**
 * Plays sound with given parameters.
 *
 * @param id Sound resource identifier
 * @param volume Volume 0.0f ... 1.0f
 * @param panning Sound panning -1.0f ... 1.0f
 * @param pitch Sound pitch 0.0f ... n
 */
void audio_play_sound(int id, float volume, float panning, float pitch);

/**
 * Starts background music playback. If there is something already playing,
 * switches to new track.
 * @param id Music file resource identifier.
 */
void audio_play_music(resource_id id);

/**
 * Stops music playback.
 */
void audio_stop_music(void);

/**
 * Set music volume
 * @param volume 0.0f ... 1.0f
 */
void audio_set_music_volume(float volume);

/**
 * Set sound volume 0.0f ... 1.0f
 * @param volume
 */
void audio_set_sound_volume(float volume);

/**
 * Get supported audio frequencies list
 */
const audio_freq *audio_get_freqs(void);

/**
 * Get supported music resamplers list
 */
const audio_mod_resampler *audio_get_resamplers(void);

#endif // AUDIO_H
