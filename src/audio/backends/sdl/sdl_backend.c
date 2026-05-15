#include "audio/backends/sdl/sdl_backend.h"
#include "audio/backends/audio_backend.h"
#include "audio/music_sources/music_source.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#include <assert.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_mixer.h>

static_assert(SDL_MIXER_VERSION_ATLEAST(2, 0, 4), "SDL_mixer version should be 2.0.4 or later");

static const audio_sample_rate supported_sample_rates[] = {
    {11025, 0, "11025Hz"},
    {22050, 0, "22050Hz"},
    {44100, 0, "44100Hz"},
    {48000, 1, "48000Hz"},
};
static const int supported_sample_rate_count = N_ELEMENTS(supported_sample_rates);

typedef struct sdl_audio_context {
    int sample_rate;
    Uint16 format;
    int channels;
    int resampler;
    float volume;
    music_source music;
    Mix_Chunk chunks[SOUND_CHANNEL_COUNT]; // Per-channel converted-sample buffers.
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
    Mix_Chunk *chunk = &ctx->chunks[i];
    if(chunk->allocated) {
        SDL_free(chunk->abuf);
        chunk->allocated = 0;
    }
}

static bool audio_get_chunk(const sdl_audio_context *ctx, Mix_Chunk *chunk, const char *src_buf, size_t src_len,
                            int src_freq, int volume) {
    Uint8 *dst_buf;
    SDL_AudioCVT cvt;

    if(SDL_BuildAudioCVT(&cvt, AUDIO_U8, 1, src_freq, ctx->format, ctx->channels, ctx->sample_rate) < 0) {
        log_error("Unable to build audio converter: %s", SDL_GetError());
        goto exit_0;
    }

    // Buffer must hold both the source bytes (copied below) and the converted output.
    if((dst_buf = SDL_malloc(src_len * cvt.len_mult + 1)) == NULL) {
        log_error("Unable to allocate memory for sound buffer");
        goto exit_0;
    }
    SDL_memcpy((void *)dst_buf, (void *)src_buf, src_len);

    cvt.buf = dst_buf;
    cvt.len = src_len;
    if(SDL_ConvertAudio(&cvt) != 0) {
        log_error("Unable to convert audio sample: %s", SDL_GetError());
        goto exit_1;
    }

    chunk->volume = volume;
    chunk->abuf = dst_buf;
    chunk->alen = cvt.len_cvt;
    chunk->allocated = 1;
    return true;

exit_1:
    SDL_free(dst_buf);
exit_0:
    return false;
}

static void set_backend_sound_volume(void *userdata, float volume) {
    assert(userdata);
    volume = clampf(volume, 0.0f, 1.0f);
    Mix_Volume(-1, volume * MIX_MAX_VOLUME);
}

static void set_backend_music_volume(void *userdata, float volume) {
    assert(userdata);
    sdl_audio_context *const ctx = userdata;
    ctx->volume = clampf(volume, 0.0f, 1.0f);
    music_source_set_volume(&ctx->music, ctx->volume);
}

static void get_info(void *userdata, unsigned *sample_rate, unsigned *channels, unsigned *resampler) {
    assert(userdata);
    const sdl_audio_context *const ctx = userdata;
    if(sample_rate != NULL) {
        *sample_rate = ctx->sample_rate;
    }
    if(channels != NULL) {
        *channels = ctx->channels;
    }
    if(resampler != NULL) {
        *resampler = ctx->resampler;
    }
}

static bool play_pcm_sound(void *userdata, int channel, const sound_source *src, int volume, int panning,
                           int fade_in_ms) {
    assert(userdata);
    assert(src);
    sdl_audio_context *const ctx = userdata;
    if(channel < 0 || channel >= SOUND_CHANNEL_COUNT) {
        return false;
    }

    // map -100 ... 100 to SDL_mixers range
    const int pan_left = (panning > 0) ? (100 - panning) * 255 / 100 : 255;
    const int pan_right = (panning < 0) ? (100 + panning) * 255 / 100 : 255;

    free_chunk(ctx, channel); // Drop any previous chunk on this channel.
    if(!audio_get_chunk(ctx, &ctx->chunks[channel], src->buf, src->len, src->freq, volume)) {
        log_error("Unable to play sound: Failed to load chunk");
        return false;
    }
    Mix_SetPanning(channel, pan_left, pan_right);
    if(Mix_FadeInChannelTimed(channel, &ctx->chunks[channel], 0, fade_in_ms, -1) == -1) {
        log_error("Unable to play sound: %s", Mix_GetError());
        return false;
    }
    return true;
}

static bool is_channel_playing(void *userdata, int channel) {
    assert(userdata);
    (void)userdata;
    if(channel < 0 || channel >= SOUND_CHANNEL_COUNT) {
        return false;
    }
    return Mix_Playing(channel) != 0;
}

static void stop_channel(void *userdata, int channel) {
    assert(userdata);
    sdl_audio_context *const ctx = userdata;
    if(channel < 0 || channel >= SOUND_CHANNEL_COUNT) {
        return;
    }
    Mix_HaltChannel(channel);
    free_chunk(ctx, channel);
}

static void fade_out_channel(void *userdata, int channel, int ms) {
    assert(userdata);
    (void)userdata;
    if(channel < 0 || channel >= SOUND_CHANNEL_COUNT) {
        return;
    }
    Mix_FadeOutChannel(channel, ms);
}

static void stop_music(void *userdata) {
    assert(userdata);
    sdl_audio_context *const ctx = userdata;
    Mix_HaltMusic();
    music_source_close(&ctx->music);
    Mix_HookMusic(NULL, NULL);
}

static void sdl_hook(void *userdata, Uint8 *stream, int len) {
    sdl_audio_context *const ctx = userdata;
    music_source_render(&ctx->music, (char *)stream, len);
}

static void play_music(void *userdata, const music_source *src) {
    assert(userdata);
    sdl_audio_context *const ctx = userdata;
    stop_music(ctx);
    memcpy(&ctx->music, src, sizeof(music_source));
    music_source_set_volume(&ctx->music, ctx->volume);
    Mix_HookMusic(sdl_hook, ctx);
}

static bool setup_backend_context(void *userdata, unsigned sample_rate, bool mono, int resampler, float music_volume,
                                  float sound_volume) {
    assert(userdata);
    sdl_audio_context *const ctx = userdata;
    memset(ctx, 0, sizeof(sdl_audio_context));

    if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        log_error("Unable to initialize audio subsystem: %s", SDL_GetError());
        goto error_0;
    }
    if(Mix_Init(0) != 0) {
        log_error("Unable to initialize mixer subsystem: %s", Mix_GetError());
        goto error_1;
    }

    log_info("Requested audio device with options:");
    log_info(" * Sample rate: %dHz", sample_rate);
    log_info(" * Channels: %d", mono ? 1 : 2);
    log_info(" * Format: %s", get_sdl_audio_format_string(AUDIO_S16SYS));

    // Request a device — the actual configuration we get back may differ.
    if(Mix_OpenAudioDevice(sample_rate, AUDIO_S16SYS, mono ? 1 : 2, 2048, NULL, 0) != 0) {
        log_error("Unable to initialize audio device: %s", SDL_GetError());
        goto error_2;
    }

    Mix_AllocateChannels(SOUND_CHANNEL_COUNT);

    set_backend_sound_volume(ctx, sound_volume);
    set_backend_music_volume(ctx, music_volume);
    ctx->resampler = resampler;

    // Query back what the device actually gave us, vs what we asked for above.
    Mix_QuerySpec(&ctx->sample_rate, &ctx->format, &ctx->channels);
    log_info("Opened audio device:");
    log_info(" * Sample rate: %dHz", ctx->sample_rate);
    log_info(" * Channels: %d", ctx->channels);
    log_info(" * Format: %s", get_sdl_audio_format_string(ctx->format));
    return true;

error_2:
    Mix_Quit();
error_1:
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
error_0:
    return false;
}

static void close_backend_context(void *userdata) {
    assert(userdata);
    sdl_audio_context *const ctx = userdata;
    log_debug("closing audio");
    stop_music(ctx);
    Mix_ChannelFinished(NULL);
    Mix_CloseAudio();
    for(int i = 0; i < SOUND_CHANNEL_COUNT; i++) {
        free_chunk(ctx, i);
    }
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void sdl_audio_backend_set_callbacks(audio_backend *sdl_backend) {
    sdl_backend->is_available = is_available;
    sdl_backend->get_description = get_description;
    sdl_backend->get_name = get_name;
    sdl_backend->get_sample_rates = get_sample_rates;
    sdl_backend->get_info = get_info;
    sdl_backend->create = create_backend;
    sdl_backend->destroy = destroy_backend;
    sdl_backend->set_music_volume = set_backend_music_volume;
    sdl_backend->set_sound_volume = set_backend_sound_volume;
    sdl_backend->setup_context = setup_backend_context;
    sdl_backend->close_context = close_backend_context;
    sdl_backend->play_pcm_sound = play_pcm_sound;
    sdl_backend->is_channel_playing = is_channel_playing;
    sdl_backend->stop_channel = stop_channel;
    sdl_backend->fade_out_channel = fade_out_channel;
    sdl_backend->play_music = play_music;
    sdl_backend->stop_music = stop_music;
}
