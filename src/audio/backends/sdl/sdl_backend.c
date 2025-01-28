#include "audio/backends/sdl/sdl_backend.h"
#include "audio/backends/audio_backend.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#include <assert.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_mixer.h>
#include <xmp.h>

#define CHANNEL_MAX 8

static const audio_sample_rate supported_sample_rates[] = {
    {11025, 0, "11025Hz"},
    {22050, 0, "22050Hz"},
    {44100, 0, "44100Hz"},
    {48000, 1, "48000Hz"},
};
static const int supported_sample_rate_count = N_ELEMENTS(supported_sample_rates);

static const audio_resampler supported_resamplers[] = {
    {XMP_INTERP_NEAREST, 0, "Nearest"},
    {XMP_INTERP_LINEAR,  1, "Linear" },
    {XMP_INTERP_SPLINE,  0, "Cubic"  },
};
static const int supported_resamplers_count = N_ELEMENTS(supported_resamplers);

typedef struct sdl_audio_context {
    int sample_rate;
    Uint16 format;
    int channels;
    int resampler;
    float music_volume;
    xmp_context xmp_context;
    Mix_Chunk channel_chunks[CHANNEL_MAX];
} sdl_audio_context;

static bool is_available(void) {
    return true; // This is always available if compiled in.
}

static const char *get_description(void) {
    return "Audio output using SDL_mixer";
}

static const char *get_name(void) {
    return "SDL_mixer";
}

static unsigned int get_sample_rates(const audio_sample_rate **sample_rates) {
    *sample_rates = supported_sample_rates;
    return supported_sample_rate_count;
}

static unsigned int get_resamplers(const audio_resampler **resamplers) {
    *resamplers = supported_resamplers;
    return supported_resamplers_count;
}

static void create_backend(audio_backend *player) {
    player->ctx = omf_calloc(1, sizeof(sdl_audio_context));
}

static void destroy_backend(audio_backend *player) {
    omf_free(player->ctx);
}

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

static inline void free_chunk(sdl_audio_context *ctx, int i) {
    if(ctx->channel_chunks[i].allocated) {
        SDL_free(ctx->channel_chunks[i].abuf);
        ctx->channel_chunks[i].allocated = 0;
    }
}

static bool audio_get_chunk(sdl_audio_context *ctx, Mix_Chunk *chunk, const char *src_buf, size_t src_len, float volume,
                            float pitch) {
    Uint8 *dst_buf;
    SDL_AudioCVT cvt;

    // Converter for sound samples.
    int src_freq = 8000 * pitch;
    if(SDL_BuildAudioCVT(&cvt, AUDIO_U8, 1, src_freq, ctx->format, ctx->channels, ctx->sample_rate) < 0) {
        PERROR("Unable to build audio converter: %s", SDL_GetError());
        goto exit_0;
    }

    // Create a buffer that can hold the source data and final converted data.
    if((dst_buf = SDL_malloc(src_len * cvt.len_mult + 1)) == NULL) {
        PERROR("Unable to allocate memory for sound buffer");
        goto exit_0;
    }
    SDL_memcpy((void *)dst_buf, (void *)src_buf, src_len);

    // Convert!
    cvt.buf = dst_buf;
    cvt.len = src_len;
    if(SDL_ConvertAudio(&cvt) != 0) {
        PERROR("Unable to convert audio sample: %s", SDL_GetError());
        goto exit_1;
    }

    chunk->volume = volume * MIX_MAX_VOLUME;
    chunk->abuf = dst_buf;
    chunk->alen = cvt.len_cvt;
    chunk->allocated = 1;
    return true;

exit_1:
    SDL_free(dst_buf);
exit_0:
    return false;
}

static bool audio_load_module(sdl_audio_context *ctx, const char *file) {
    assert(ctx);

    if(!ctx->xmp_context) {
        if((ctx->xmp_context = xmp_create_context()) == NULL) {
            PERROR("Unable to initialize XMP context.");
            goto exit_0;
        }
    }

    // Load the module file
    if(xmp_load_module(ctx->xmp_context, (char *)file) < 0) {
        PERROR("Unable to open module file");
        goto exit_0;
    }

    // Show some information
    struct xmp_module_info mi;
    xmp_get_module_info(ctx->xmp_context, &mi);
    DEBUG("Loaded music track %s (%s)", mi.mod->name, mi.mod->type);

    // Start the player
    int flags = 0;
    if(ctx->channels == 1)
        flags |= XMP_FORMAT_MONO;
    if(xmp_start_player(ctx->xmp_context, ctx->sample_rate, flags) != 0) {
        PERROR("Unable to start module playback");
        goto exit_1;
    }
    if(xmp_set_player(ctx->xmp_context, XMP_PLAYER_INTERP, ctx->resampler) != 0) {
        PERROR("Unable to set music resampler");
        goto exit_2;
    }
    if(xmp_set_player(ctx->xmp_context, XMP_PLAYER_VOLUME, ctx->music_volume * 100) != 0) {
        PERROR("Unable to set music volume");
        goto exit_2;
    }
    return true;

exit_2:
    xmp_end_player(ctx->xmp_context);
exit_1:
    xmp_release_module(ctx->xmp_context);
exit_0:
    return false;
}

// Callback function for SDL_Mixer
void audio_xmp_render(void *userdata, Uint8 *stream, int len) {
    sdl_audio_context *ctx = userdata;
    assert(ctx);
    assert(ctx->xmp_context);
    xmp_play_buffer(ctx->xmp_context, stream, len, 0);
}

static void audio_close_module(sdl_audio_context *ctx) {
    if(ctx->xmp_context != NULL) {
        xmp_end_player(ctx->xmp_context);
        xmp_release_module(ctx->xmp_context);
        xmp_free_context(ctx->xmp_context);
        ctx->xmp_context = NULL;
    }
}

static void set_backend_sound_volume(void *userdata, float volume) {
    assert(userdata);
    volume = clampf(volume, VOLUME_MIN, VOLUME_MAX);
    Mix_Volume(-1, volume * MIX_MAX_VOLUME);
}

static void set_backend_music_volume(void *userdata, float volume) {
    assert(userdata);
    sdl_audio_context *ctx = userdata;
    ctx->music_volume = clampf(volume, VOLUME_MIN, VOLUME_MAX);
    xmp_set_player(ctx->xmp_context, XMP_PLAYER_VOLUME, ctx->music_volume * 100);
}

static void play_sound(void *userdata, const char *src_buf, size_t src_len, float volume, float panning, float pitch) {
    assert(userdata);
    sdl_audio_context *ctx = userdata;

    int channel;
    float pan_left, pan_right;

    volume = clampf(volume, VOLUME_MIN, VOLUME_MAX);
    panning = clampf(panning, PANNING_MIN, PANNING_MAX);
    pitch = clampf(pitch, PITCH_MIN, PITCH_MAX);
    pan_left = (panning > 0) ? 1.0f - panning : 1.0f;
    pan_right = (panning < 0) ? 1.0f + panning : 1.0f;

    if((channel = Mix_GroupAvailable(-1)) == -1) {
        return;
    }
    free_chunk(ctx, channel); // Make sure old chunk is deallocated, if one exists.
    if(!audio_get_chunk(ctx, &ctx->channel_chunks[channel], src_buf, src_len, volume, pitch)) {
        PERROR("Unable to play sound: Failed to load chunk");
        return;
    }
    Mix_SetPanning(channel, clamp(pan_left * 255, 0, 255), clamp(pan_right * 255, 0, 255));
    if(Mix_PlayChannel(channel, &ctx->channel_chunks[channel], 0) == -1) {
        PERROR("Unable to play sound: %s", Mix_GetError());
    }
}

static void stop_music(void *ctx) {
    assert(ctx);
    Mix_HaltMusic();
    Mix_HookMusic(NULL, NULL);
}

static void play_music(void *userdata, const char *file_name) {
    assert(userdata);
    sdl_audio_context *ctx = userdata;
    stop_music(ctx);
    audio_close_module(ctx);
    if(!audio_load_module(ctx, file_name)) {
        PERROR("Unable to load music track: %s", file_name);
        return;
    }
    Mix_HookMusic(audio_xmp_render, ctx);
}

static bool setup_backend_context(void *userdata, unsigned sample_rate, bool mono, unsigned resampler,
                                  float music_volume, float sound_volume) {
    assert(userdata);
    sdl_audio_context *ctx = userdata;
    memset(ctx, 0, sizeof(sdl_audio_context));

    if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        PERROR("Unable to initialize audio subsystem: %s", SDL_GetError());
        goto error_0;
    }
    if(Mix_Init(0) != 0) {
        PERROR("Unable to initialize mixer subsystem: %s", Mix_GetError());
        goto error_1;
    }
    if((ctx->xmp_context = xmp_create_context()) == NULL) {
        PERROR("Unable to initialize XMP context.");
        goto error_2;
    }

    INFO("Requested audio device with options:");
    INFO(" * Sample rate: %dHz", sample_rate);
    INFO(" * Channels: %d", mono ? 1 : 2);
    INFO(" * Format: %s", get_sdl_audio_format_string(AUDIO_S16SYS));

    // Setup audio. We request for configuration, but we're not sure what we get.
    if(Mix_OpenAudioDevice(sample_rate, AUDIO_S16SYS, mono ? 1 : 2, 2048, NULL, 0) != 0) {
        PERROR("Unable to initialize audio device: %s", SDL_GetError());
        goto error_3;
    }

    // Make sure we have the correct amount of channels.
    Mix_AllocateChannels(CHANNEL_MAX);

    // Initialize playback parameters.
    set_backend_sound_volume(ctx, sound_volume);
    set_backend_music_volume(ctx, music_volume);
    ctx->resampler = resampler;

    // Get the actual device configuration we got.
    Mix_QuerySpec(&ctx->sample_rate, &ctx->format, &ctx->channels);
    INFO("Opened audio device:");
    INFO(" * Sample rate: %dHz", ctx->sample_rate);
    INFO(" * Channels: %d", ctx->channels);
    INFO(" * Format: %s", get_sdl_audio_format_string(ctx->format));
    return true;

error_3:
    xmp_free_context(ctx->xmp_context);
error_2:
    Mix_Quit();
error_1:
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
error_0:
    return false;
}

static void close_backend_context(void *userdata) {
    assert(userdata);
    sdl_audio_context *ctx = userdata;
    DEBUG("closing audio");
    stop_music(ctx);
    audio_close_module(ctx);
    Mix_ChannelFinished(NULL);
    Mix_CloseAudio();
    for(int i = 0; i < CHANNEL_MAX; i++) {
        free_chunk(ctx, i);
    }
    if(ctx->xmp_context) {
        xmp_free_context(ctx->xmp_context);
        ctx->xmp_context = NULL;
    }
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void sdl_audio_backend_set_callbacks(audio_backend *sdl_backend) {
    sdl_backend->is_available = is_available;
    sdl_backend->get_description = get_description;
    sdl_backend->get_name = get_name;
    sdl_backend->get_sample_rates = get_sample_rates;
    sdl_backend->get_resamplers = get_resamplers;
    sdl_backend->create = create_backend;
    sdl_backend->destroy = destroy_backend;
    sdl_backend->set_music_volume = set_backend_music_volume;
    sdl_backend->set_sound_volume = set_backend_sound_volume;
    sdl_backend->setup_context = setup_backend_context;
    sdl_backend->close_context = close_backend_context;
    sdl_backend->play_music = play_music;
    sdl_backend->play_sound = play_sound;
    sdl_backend->stop_music = stop_music;
}
