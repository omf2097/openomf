#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

#include "audio/backends/audio_backend.h"
#include "resources/ids.h"

void audio_scan_backends(void);
bool audio_get_backend_info(int index, const char **name, const char **description);
int audio_get_backend_count(void);

/**
 * Initializes the audio subsystem
 *
 * @param try_name Audio player backend to use
 * @param sample_rate Wanted output frequency (48000 should be fine)
 * @param mono True if 1 channel, False for 2.
 * @param resampler Music module resampler interpolation
 * @param music_volume Initial music volume
 * @param sound_volume Initial audio volume
 * @return True if initialized, false if not.
 */
bool audio_init(const char *try_name, int sample_rate, bool mono, int resampler, float music_volume,
                float sound_volume);

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
 * @return backend specific reference ID to the playing sound for later use with audio_fade_out or -1 on failure
 */
int audio_play_sound(int id, float volume, float panning, float pitch);

/**
 * Plays sound with given parameters from a buffer.
 *
 * @param src_buf Sound data buffer
 * @param src_len Sound data buffer length
 * @param volume Volume 0.0f ... 1.0f
 * @param panning Sound panning -1.0f ... 1.0f
 * @param pitch Sound pitch 0.0f ... n
 * @param fade How many milliseconds to fade in the playback over
 * @return backend specific reference ID to the playing sound for later use with audio_fade_out or -1 on failure
 */
int audio_play_sound_buf(char *src_buf, int src_len, float volume, float panning, float pitch, int fade);

/**
 * Fade out audio already playing
 *
 * @param playback_id The playback handle returned from a previous call to audio_play_sound or audio_play_sound_buf
 * @param ms How many milliseconds to fade to silence over
 */
void audio_fade_out(int playback_id, int ms);

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
unsigned audio_get_sample_rates(const audio_sample_rate **sample_rates);

/**
 * Get supported music resamplers list
 */
unsigned audio_get_resamplers(const audio_resampler **resamplers);

/**
 * Calculate sample rate after applying a pitch
 */
int pitched_samplerate(float pitch);

#endif // AUDIO_H
