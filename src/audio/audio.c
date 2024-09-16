#include <assert.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_mixer.h>
#include <xmp.h>

#include "audio/audio.h"
#include "resources/pathmanager.h"
#include "resources/sounds_loader.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#define CHANNEL_MAX 16

const audio_freq output_freqs[] = {
    {11025, 0, "11025Hz"},
    {22050, 0, "22050Hz"},
    {44100, 0, "44100Hz"},
    {48000, 1, "48000Hz"},
    {0,     0, 0        }  // Guard
};

const audio_mod_resampler music_resamplers[] = {
    {XMP_INTERP_NEAREST, 0, "Nearest"},
    {XMP_INTERP_LINEAR,  1, "Linear" },
    {XMP_INTERP_SPLINE,  0, "Cubic"  },
    {0,                  0, 0        }  // Guard
};

typedef struct audio_system {
    int freq;
    Uint16 format;
    int channels;
    int resampler;
    float music_volume;
    resource_id music_id;
    xmp_context xmp_context;
    Mix_Chunk *channel_chunks[CHANNEL_MAX];
} audio_system;

static audio_system *audio = NULL;

static const char *get_sdl_audio_format_string(SDL_AudioFormat format) {
    switch(format) {
        case AUDIO_U8:
            return "AUDIO_U8";
        case AUDIO_S8:
            return "AUDIO_S8";
        case AUDIO_U16LSB:
            return "AUDIO_U16LSB";
        case AUDIO_S16LSB:
            return "AUDIO_S16LSB";
        case AUDIO_U16MSB:
            return "AUDIO_U16MSB";
        case AUDIO_S16MSB:
            return "AUDIO_S16MSB";
        case AUDIO_S32LSB:
            return "AUDIO_S32LSB";
        case AUDIO_S32MSB:
            return "AUDIO_S32MSB";
        case AUDIO_F32LSB:
            return "AUDIO_F32LSB";
        case AUDIO_F32MSB:
            return "AUDIO_F32MSB";
    }
    return "UNKNOWN";
}

static Mix_Chunk *audio_get_chunk(int id, float volume, float pitch) {
    char *src_buf;
    int src_len;
    Uint8 *dst_buf;
    SDL_AudioCVT cvt;

    // Load sample (8000Hz, mono, 8bit)
    if(sounds_loader_get(id, &src_buf, &src_len) != 0) {
        PERROR("Requested sound sample %d not found", id);
        return NULL;
    }
    if(src_len == 0) {
        DEBUG("Requested sound sample %d has nothing to play", id);
        return NULL;
    }

    // Converter for sound samples.
    int src_freq = 8000 * pitch;
    if(SDL_BuildAudioCVT(&cvt, AUDIO_U8, 1, src_freq, audio->format, audio->channels, audio->freq) < 0) {
        PERROR("Unable to build audio converter: %s", SDL_GetError());
        return NULL;
    }

    // Create a buffer that can hold the source data and final converted data.
    if((dst_buf = SDL_malloc(src_len * cvt.len_mult + 1)) == NULL) {
        PERROR("Unable to allocate memory for sound buffer");
        return NULL;
    }
    SDL_memcpy((void *)dst_buf, (void *)src_buf, src_len);

    // Convert!
    cvt.buf = dst_buf;
    cvt.len = src_len;
    if(SDL_ConvertAudio(&cvt) != 0) {
        PERROR("Unable to convert audio sample: %s", SDL_GetError());
        return NULL;
    }

    Mix_Chunk *chunk = SDL_malloc(sizeof(Mix_Chunk));
    chunk->volume = volume * MIX_MAX_VOLUME;
    chunk->abuf = dst_buf;
    chunk->alen = cvt.len_cvt;
    chunk->allocated = 1;
    return chunk;
}

static void audio_sound_finished(int channel) {
    assert(audio);
    if(audio->channel_chunks[channel] != NULL) {
        Mix_FreeChunk(audio->channel_chunks[channel]);
        audio->channel_chunks[channel] = NULL;
    }
}

static bool audio_load_module(const char *file) {
    assert(audio);

    if (!audio->xmp_context) {
        if((audio->xmp_context = xmp_create_context()) == NULL) {
            PERROR("Unable to initialize XMP context.");
            goto exit_0;
        }
    }

    // Load the module file
    if(xmp_load_module(audio->xmp_context, (char *)file) < 0) {
        PERROR("Unable to open module file");
        goto exit_0;
    }

    // Show some information
    struct xmp_module_info mi;
    xmp_get_module_info(audio->xmp_context, &mi);
    DEBUG("Loaded music track %s (%s)", mi.mod->name, mi.mod->type);

    // Start the player
    int flags = 0;
    if(audio->channels == 1)
        flags |= XMP_FORMAT_MONO;
    if(xmp_start_player(audio->xmp_context, audio->freq, flags) != 0) {
        PERROR("Unable to start module playback");
        goto exit_1;
    }
    if(xmp_set_player(audio->xmp_context, XMP_PLAYER_INTERP, audio->resampler) != 0) {
        PERROR("Unable to set music resampler");
        goto exit_2;
    }
    if(xmp_set_player(audio->xmp_context, XMP_PLAYER_VOLUME, audio->music_volume * 100) != 0) {
        PERROR("Unable to set music volume");
        goto exit_2;
    }
    return true;

exit_2:
    xmp_end_player(audio->xmp_context);
exit_1:
    xmp_release_module(audio->xmp_context);
exit_0:
    return false;
}

// Callback function for SDL_Mixer
void audio_xmp_render(void *userdata, Uint8 *stream, int len) {
    assert(audio);
    assert(audio->xmp_context);
    xmp_play_buffer(audio->xmp_context, stream, len, 0);
}

static void audio_close_module() {
    if(is_music(audio->music_id)) {
        xmp_end_player(audio->xmp_context);
        xmp_release_module(audio->xmp_context);
        xmp_free_context(audio->xmp_context);
        audio->xmp_context = NULL;
        audio->music_id = NUMBER_OF_RESOURCES;
    }
}

bool audio_init(int freq, bool mono, int resampler, float music_volume, float sound_volume) {
    if(!(audio = omf_calloc(1, sizeof(audio_system)))) {
        PERROR("Unable to allocate audio subsystem");
        goto error_0;
    }
    audio->xmp_context = NULL;
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        PERROR("Unable to initialize audio subsystem: %s", SDL_GetError());
        goto error_1;
    }
    if(Mix_Init(0) != 0) {
        PERROR("Unable to initialize mixer subsystem: %s", Mix_GetError());
        goto error_2;
    }
    if((audio->xmp_context = xmp_create_context()) == NULL) {
        PERROR("Unable to initialize XMP context.");
        goto error_3;
    }

    INFO("Requested audio device with options:");
    INFO(" * Rate: %dHz", freq);
    INFO(" * Channels: %d", mono ? 1 : 2);
    INFO(" * Format: %s", get_sdl_audio_format_string(AUDIO_S16SYS));

    // Setup audio. We request for configuration, but we're not sure what we get.
    if(Mix_OpenAudio(freq, AUDIO_S16SYS, mono ? 1 : 2, 2048) != 0) {
        PERROR("Unable to initialize audio device: %s", SDL_GetError());
        goto error_4;
    }

    // Make sure we have the correct amount of channels.
    Mix_AllocateChannels(CHANNEL_MAX);

    // Initialize playback parameters.
    audio_set_sound_volume(sound_volume);
    audio_set_music_volume(music_volume);
    audio->resampler = resampler;
    audio->music_id = NUMBER_OF_RESOURCES;

    Mix_ChannelFinished(audio_sound_finished);

    // Get the actual device configuration we got.
    Mix_QuerySpec(&audio->freq, &audio->format, &audio->channels);
    INFO("Opened audio device:");
    INFO(" * Rate: %dHz", audio->freq);
    INFO(" * Channels: %d", audio->channels);
    INFO(" * Format: %s", get_sdl_audio_format_string(audio->format));
    return true;

error_4:
    xmp_free_context(audio->xmp_context);
error_3:
    Mix_Quit();
error_2:
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
error_1:
    omf_free(audio);
error_0:
    return false;
}

void audio_close() {
    if(audio != NULL) {
        DEBUG("closing audio");
        audio_stop_music();
        audio_close_module();
        Mix_ChannelFinished(NULL);
        Mix_CloseAudio();
        for(int i = 0; i < CHANNEL_MAX; i++) {
            if(audio->channel_chunks[i] != NULL) {
                Mix_FreeChunk(audio->channel_chunks[i]);
                audio->channel_chunks[i] = NULL;
            }
        }
        if(audio->xmp_context) {
            xmp_free_context(audio->xmp_context);
            audio->xmp_context = NULL;
        }
        omf_free(audio);
    }
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void audio_play_sound(int id, float volume, float panning, float pitch) {
    assert(audio);
    int channel;
    Mix_Chunk *chunk;
    float pan_left, pan_right;

    // Anything beyond these are invalid
    if(id < 0 || id > 299)
        goto error_0;

    volume = clampf(volume, VOLUME_MIN, VOLUME_MAX);
    panning = clampf(panning, PANNING_MIN, PANNING_MAX);
    pitch = clampf(pitch, PITCH_MIN, PITCH_MAX);
    pan_left = (panning > 0) ? 1.0f - panning : 1.0f;
    pan_right = (panning < 0) ? 1.0f + panning : 1.0f;

    if((channel = Mix_GroupAvailable(-1)) == -1) {
        // PERROR("Unable to play sound: No free channels");
        goto error_0;
    }
    if((chunk = audio_get_chunk(id, volume, pitch)) == NULL) {
        PERROR("Unable to play sound: Failed to load chunk");
        goto error_0;
    }
    Mix_SetPanning(channel, clamp(pan_left * 255, 0, 255), clamp(pan_right * 255, 0, 255));
    if(Mix_PlayChannel(channel, chunk, 0) == -1) {
        PERROR("Unable to play sound: %s", Mix_GetError());
        goto error_1;
    }
    audio->channel_chunks[channel] = chunk;
    return;

error_1:
    free(chunk);
error_0:
    return;
}

void audio_play_music(resource_id id) {
    assert(audio);
    assert(is_music(id));
    if(audio->music_id != id) {
        audio_stop_music();
        audio_close_module();
        const char *music_file = pm_get_resource_path(id);
        if(!audio_load_module(music_file)) {
            PERROR("Unable to load music track: %s", music_file);
            return;
        }
        audio->music_id = id;
        Mix_HookMusic(audio_xmp_render, NULL);
    }
}

void audio_stop_music() {
    assert(audio);
    Mix_HaltMusic();
    Mix_HookMusic(NULL, NULL);
}

void audio_set_music_volume(float volume) {
    assert(audio);
    audio->music_volume = clampf(volume, VOLUME_MIN, VOLUME_MAX);
    xmp_set_player(audio->xmp_context, XMP_PLAYER_VOLUME, audio->music_volume * 100);
}

void audio_set_sound_volume(float volume) {
    assert(audio);
    volume = clampf(volume, VOLUME_MIN, VOLUME_MAX);
    Mix_Volume(-1, volume * MIX_MAX_VOLUME);
}

const audio_freq *audio_get_freqs() {
    return output_freqs;
}

const audio_mod_resampler *audio_get_resamplers() {
    return music_resamplers;
}
