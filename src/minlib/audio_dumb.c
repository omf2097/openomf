#ifndef MIN_LIB_BUILD
#error "audio_dumb.c is part of MIN_LIB_BUILD"
#endif

#include "minlib/audio_dumb.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <mmsystem.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define WAVE_BUFS 4
#define WAVE_FRAMES 2048
#define MIX_MAX_VOLUME 128

typedef struct dumb_chan {
    int16_t *pcm;
    int frames;
    int pos;
    int volume;
    int pan_left;
    int pan_right;
    int playing;
    int fade_total;
    int fade_done;
    int fade_out;
} dumb_chan;

typedef struct dumb_audio_context {
    int sample_rate;
    int channels;
    int resampler;
    int master_volume;
    float music_volume;
    int active;
    int open;
    music_source music;
    dumb_chan chans[SOUND_CHANNEL_COUNT];
    CRITICAL_SECTION lock;
    int lock_ready;
    HWAVEOUT wave;
    WAVEHDR hdrs[WAVE_BUFS];
    uint8_t *bufs[WAVE_BUFS];
    int buf_bytes;
} dumb_audio_context;

static const audio_sample_rate supported_sample_rates[] = {
    {11025, 0, "11025Hz"},
    {22050, 0, "22050Hz"},
    {44100, 0, "44100Hz"},
    {48000, 1, "48000Hz"},
};
static const int supported_sample_rate_count = N_ELEMENTS(supported_sample_rates);

static int clamp16(int value) {
    if(value > 32767) {
        return 32767;
    }
    if(value < -32768) {
        return -32768;
    }
    return value;
}

static void lock_audio(dumb_audio_context *ctx) {
    if(ctx->lock_ready) {
        EnterCriticalSection(&ctx->lock);
    }
}

static void unlock_audio(dumb_audio_context *ctx) {
    if(ctx->lock_ready) {
        LeaveCriticalSection(&ctx->lock);
    }
}

static void free_channel(dumb_chan *chan) {
    free(chan->pcm);
    chan->pcm = NULL;
    chan->frames = 0;
    chan->pos = 0;
    chan->playing = 0;
    chan->fade_total = 0;
    chan->fade_done = 0;
    chan->fade_out = 0;
}

/* Effects are unsigned 8-bit mono. Device samples are signed 16-bit. */
static int16_t *convert_effect(const char *src, int src_frames, int src_rate, int dst_rate, int dst_channels,
                               int *dst_frames) {
    float *decoded;
    int16_t *dst;
    int frames;
    int i;
    if(src == NULL || src_frames <= 0 || src_rate <= 0 || dst_rate <= 0 || dst_channels <= 0) {
        return NULL;
    }
    frames = (int)((double)src_frames * (double)dst_rate / (double)src_rate);
    if(frames < 1) {
        frames = 1;
    }
    decoded = (float *)malloc((size_t)src_frames * sizeof(float));
    dst = (int16_t *)malloc((size_t)frames * (size_t)dst_channels * sizeof(int16_t));
    if(decoded == NULL || dst == NULL) {
        free(decoded);
        free(dst);
        return NULL;
    }
    for(i = 0; i < src_frames; i++) {
        decoded[i] = ((int)(uint8_t)src[i] - 128) / 128.0f;
    }
    for(i = 0; i < frames; i++) {
        double pos = (src_frames <= 1) ? 0.0 : ((double)i * (double)(src_frames - 1) / (double)frames);
        int i0 = (int)pos;
        int i1 = i0 + 1;
        float t = (float)(pos - (double)i0);
        float sample;
        int c;
        if(i0 >= src_frames) {
            i0 = src_frames - 1;
        }
        if(i1 >= src_frames) {
            i1 = src_frames - 1;
        }
        sample = decoded[i0] + (decoded[i1] - decoded[i0]) * t;
        if(sample > 1.0f) {
            sample = 1.0f;
        }
        if(sample < -1.0f) {
            sample = -1.0f;
        }
        for(c = 0; c < dst_channels; c++) {
            dst[i * dst_channels + c] = (int16_t)(sample * 32767.0f);
        }
    }
    free(decoded);
    *dst_frames = frames;
    return dst;
}

static void mix_block(dumb_audio_context *ctx, uint8_t *dst, int nbytes) {
    int frames;
    int i;
    int ch;
    int16_t *out;
    if(nbytes <= 0 || ctx->channels <= 0) {
        return;
    }
    memset(dst, 0, (size_t)nbytes);
    if(ctx->music.context != NULL) {
        music_source_render(&ctx->music, (char *)dst, nbytes);
    }
    frames = nbytes / (ctx->channels * (int)sizeof(int16_t));
    out = (int16_t *)dst;
    for(ch = 0; ch < SOUND_CHANNEL_COUNT; ch++) {
        dumb_chan *c = &ctx->chans[ch];
        if(!c->playing || c->pcm == NULL) {
            continue;
        }
        for(i = 0; i < frames; i++) {
            float gain = 1.0f;
            int left;
            int right;
            if(!c->playing || c->pos >= c->frames) {
                c->playing = 0;
                break;
            }
            if(c->fade_total > 0) {
                gain = (float)c->fade_done / (float)c->fade_total;
                if(gain > 1.0f) {
                    gain = 1.0f;
                }
                if(c->fade_out) {
                    gain = 1.0f - gain;
                }
                c->fade_done++;
                if(c->fade_done >= c->fade_total) {
                    c->fade_total = 0;
                    if(c->fade_out) {
                        c->playing = 0;
                    }
                }
            }
            left = (int)((float)(c->pcm[c->pos * ctx->channels] * ctx->master_volume * c->volume /
                                 (MIX_MAX_VOLUME * MIX_MAX_VOLUME)) *
                         gain);
            left = left * c->pan_left / 255;
            if(ctx->channels >= 2) {
                right = (int)((float)(c->pcm[c->pos * ctx->channels + 1] * ctx->master_volume * c->volume /
                                      (MIX_MAX_VOLUME * MIX_MAX_VOLUME)) *
                              gain);
                right = right * c->pan_right / 255;
                out[i * ctx->channels] = (int16_t)clamp16((int)out[i * ctx->channels] + left);
                out[i * ctx->channels + 1] = (int16_t)clamp16((int)out[i * ctx->channels + 1] + right);
            } else {
                out[i] = (int16_t)clamp16((int)out[i] + left);
            }
            c->pos++;
        }
    }
}

static void CALLBACK wave_cb(HWAVEOUT hwo, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2) {
    dumb_audio_context *ctx = (dumb_audio_context *)instance;
    WAVEHDR *hdr;
    int write_back = 0;
    (void)param2;
    if(msg != WOM_DONE || ctx == NULL) {
        return;
    }
    hdr = (WAVEHDR *)param1;
    lock_audio(ctx);
    if(ctx->open && hdr != NULL && hdr->lpData != NULL) {
        mix_block(ctx, (uint8_t *)hdr->lpData, (int)hdr->dwBufferLength);
        write_back = 1;
    }
    unlock_audio(ctx);
    if(write_back) {
        waveOutWrite(hwo, hdr, sizeof(*hdr));
    }
}

static void release_device(dumb_audio_context *ctx) {
    int i;
    if(ctx->wave == NULL) {
        return;
    }
    waveOutReset(ctx->wave);
    for(i = 0; i < WAVE_BUFS; i++) {
        if(ctx->hdrs[i].lpData != NULL) {
            waveOutUnprepareHeader(ctx->wave, &ctx->hdrs[i], sizeof(ctx->hdrs[i]));
        }
        free(ctx->bufs[i]);
        ctx->bufs[i] = NULL;
        ctx->hdrs[i].lpData = NULL;
    }
    waveOutClose(ctx->wave);
    ctx->wave = NULL;
}

static bool is_available(void) {
    return true;
}

static const char *get_description(void) {
    return "Audio output using waveOut";
}

static const char *get_name(void) {
    return "waveOut";
}

static unsigned int get_sample_rates(const audio_sample_rate **sample_rates) {
    *sample_rates = supported_sample_rates;
    return supported_sample_rate_count;
}

static void create_backend(audio_backend *player) {
    player->ctx = omf_calloc(1, sizeof(dumb_audio_context));
}

static void set_backend_sound_volume(void *userdata, float volume) {
    dumb_audio_context *ctx = userdata;
    assert(ctx);
    lock_audio(ctx);
    ctx->master_volume = (int)(clampf(volume, 0.0f, 1.0f) * (float)MIX_MAX_VOLUME);
    unlock_audio(ctx);
}

static void set_backend_music_volume(void *userdata, const float volume) {
    dumb_audio_context *ctx = userdata;
    assert(ctx);
    lock_audio(ctx);
    ctx->music_volume = clampf(volume, 0.0f, 1.0f);
    music_source_set_volume(&ctx->music, ctx->music_volume);
    unlock_audio(ctx);
}

static void get_info(void *userdata, unsigned *sample_rate, unsigned *channels, unsigned *resampler) {
    const dumb_audio_context *ctx = userdata;
    assert(ctx);
    if(sample_rate != NULL) {
        *sample_rate = (unsigned)ctx->sample_rate;
    }
    if(channels != NULL) {
        *channels = (unsigned)ctx->channels;
    }
    if(resampler != NULL) {
        *resampler = (unsigned)ctx->resampler;
    }
}

static void stop_music(void *userdata) {
    dumb_audio_context *ctx = userdata;
    music_source previous;
    assert(ctx);
    lock_audio(ctx);
    previous = ctx->music;
    memset(&ctx->music, 0, sizeof(ctx->music));
    unlock_audio(ctx);
    music_source_close(&previous);
}

static void close_backend_context(void *userdata) {
    dumb_audio_context *ctx = userdata;
    int i;
    assert(ctx);
    if(!ctx->active) {
        return;
    }
    log_debug("closing audio");
    lock_audio(ctx);
    ctx->open = 0;
    unlock_audio(ctx);
    release_device(ctx);
    lock_audio(ctx);
    for(i = 0; i < SOUND_CHANNEL_COUNT; i++) {
        free_channel(&ctx->chans[i]);
    }
    unlock_audio(ctx);
    stop_music(ctx);
    if(ctx->lock_ready) {
        DeleteCriticalSection(&ctx->lock);
        ctx->lock_ready = 0;
    }
    ctx->active = 0;
}

static void destroy_backend(audio_backend *player) {
    if(player->ctx != NULL) {
        close_backend_context(player->ctx);
        omf_free(player->ctx);
        player->ctx = NULL;
    }
}

static bool setup_backend_context(void *userdata, const unsigned sample_rate, const bool mono, const int resampler,
                                  const float music_volume, const float sound_volume) {
    dumb_audio_context *ctx = userdata;
    WAVEFORMATEX fmt;
    MMRESULT result;
    int channels;
    int i;
    assert(ctx);
    if(sample_rate < 8000) {
        log_error("Refusing to open waveOut at %uHz", sample_rate);
        return false;
    }
    channels = mono ? 1 : 2;
    memset(ctx, 0, sizeof(*ctx));
    InitializeCriticalSection(&ctx->lock);
    ctx->lock_ready = 1;
    ctx->sample_rate = (int)sample_rate;
    ctx->channels = channels;
    ctx->resampler = resampler;

    log_info("Requested audio device with options:");
    log_info(" * Sample rate: %uHz", sample_rate);
    log_info(" * Channels: %d", channels);
    log_info(" * Format: AUDIO_S16SYS");

    memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = (WORD)channels;
    fmt.nSamplesPerSec = sample_rate;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = (WORD)(channels * 2);
    fmt.nAvgBytesPerSec = sample_rate * fmt.nBlockAlign;
    result = waveOutOpen(&ctx->wave, WAVE_MAPPER, &fmt, (DWORD_PTR)wave_cb, (DWORD_PTR)ctx, CALLBACK_FUNCTION);
    if(result != MMSYSERR_NOERROR) {
        log_error("waveOutOpen failed (%u)", (unsigned)result);
        goto fail;
    }
    ctx->buf_bytes = WAVE_FRAMES * (int)fmt.nBlockAlign;
    for(i = 0; i < WAVE_BUFS; i++) {
        ctx->bufs[i] = (uint8_t *)calloc(1, (size_t)ctx->buf_bytes);
        if(ctx->bufs[i] == NULL) {
            log_error("out of memory for audio buffers");
            goto fail;
        }
        memset(&ctx->hdrs[i], 0, sizeof(ctx->hdrs[i]));
        ctx->hdrs[i].lpData = (LPSTR)ctx->bufs[i];
        ctx->hdrs[i].dwBufferLength = (DWORD)ctx->buf_bytes;
        if(waveOutPrepareHeader(ctx->wave, &ctx->hdrs[i], sizeof(ctx->hdrs[i])) != MMSYSERR_NOERROR) {
            log_error("waveOutPrepareHeader failed");
            goto fail;
        }
    }
    for(i = 0; i < SOUND_CHANNEL_COUNT; i++) {
        ctx->chans[i].pan_left = 255;
        ctx->chans[i].pan_right = 255;
    }
    ctx->active = 1;
    ctx->open = 1;
    set_backend_sound_volume(ctx, sound_volume);
    set_backend_music_volume(ctx, music_volume);
    for(i = 0; i < WAVE_BUFS; i++) {
        waveOutWrite(ctx->wave, &ctx->hdrs[i], sizeof(ctx->hdrs[i]));
    }
    log_info("Opened audio device:");
    log_info(" * Sample rate: %dHz", ctx->sample_rate);
    log_info(" * Channels: %d", ctx->channels);
    log_info(" * Format: AUDIO_S16SYS");
    return true;

fail:
    ctx->open = 0;
    release_device(ctx);
    DeleteCriticalSection(&ctx->lock);
    memset(ctx, 0, sizeof(*ctx));
    return false;
}

static bool play_pcm_sound(void *userdata, const int channel, const sound_source *src, const int volume,
                           const int panning, const int fade_in_ms) {
    dumb_audio_context *ctx = userdata;
    int16_t *pcm;
    int frames = 0;
    int src_frames;
    int pan_left;
    int pan_right;
    assert(ctx);
    assert(src);
    if(!ctx->active || channel < 0 || channel >= SOUND_CHANNEL_COUNT || src->buf == NULL || src->freq <= 0) {
        return false;
    }
    if(src->len == 0 || src->len > 0x7fffffff) {
        return false;
    }
    src_frames = (int)src->len;
    pcm = convert_effect(src->buf, src_frames, src->freq, ctx->sample_rate, ctx->channels, &frames);
    if(pcm == NULL) {
        log_error("Unable to play sound: failed to convert sample");
        return false;
    }
    pan_left = (panning > 0) ? (100 - panning) * 255 / 100 : 255;
    pan_right = (panning < 0) ? (100 + panning) * 255 / 100 : 255;
    lock_audio(ctx);
    free_channel(&ctx->chans[channel]);
    ctx->chans[channel].pcm = pcm;
    ctx->chans[channel].frames = frames;
    ctx->chans[channel].pos = 0;
    ctx->chans[channel].volume = clamp(volume, 0, 127);
    ctx->chans[channel].pan_left = pan_left;
    ctx->chans[channel].pan_right = pan_right;
    ctx->chans[channel].playing = 1;
    ctx->chans[channel].fade_done = 0;
    ctx->chans[channel].fade_out = 0;
    ctx->chans[channel].fade_total =
        (fade_in_ms > 0 && ctx->sample_rate > 0) ? (fade_in_ms * ctx->sample_rate / 1000) : 0;
    unlock_audio(ctx);
    return true;
}

static bool is_channel_playing(void *userdata, const int channel) {
    dumb_audio_context *ctx = userdata;
    int playing;
    assert(ctx);
    if(channel < 0 || channel >= SOUND_CHANNEL_COUNT) {
        return false;
    }
    lock_audio(ctx);
    playing = ctx->chans[channel].playing;
    unlock_audio(ctx);
    return playing != 0;
}

static void stop_channel(void *userdata, const int channel) {
    dumb_audio_context *ctx = userdata;
    assert(ctx);
    if(channel < 0 || channel >= SOUND_CHANNEL_COUNT) {
        return;
    }
    lock_audio(ctx);
    free_channel(&ctx->chans[channel]);
    ctx->chans[channel].pan_left = 255;
    ctx->chans[channel].pan_right = 255;
    unlock_audio(ctx);
}

static void fade_out_channel(void *userdata, const int channel, const int ms) {
    dumb_audio_context *ctx = userdata;
    assert(ctx);
    if(channel < 0 || channel >= SOUND_CHANNEL_COUNT) {
        return;
    }
    lock_audio(ctx);
    if(ctx->chans[channel].playing) {
        if(ms <= 0) {
            ctx->chans[channel].playing = 0;
        } else {
            int frames = (ctx->sample_rate > 0) ? (ms * ctx->sample_rate / 1000) : 1;
            if(frames < 1) {
                frames = 1;
            }
            ctx->chans[channel].fade_out = 1;
            ctx->chans[channel].fade_done = 0;
            ctx->chans[channel].fade_total = frames;
        }
    }
    unlock_audio(ctx);
}

static void set_channel_panning(void *userdata, const int channel, const int panning) {
    dumb_audio_context *ctx = userdata;
    assert(ctx);
    if(channel < 0 || channel >= SOUND_CHANNEL_COUNT) {
        return;
    }
    lock_audio(ctx);
    ctx->chans[channel].pan_left = (panning > 0) ? (100 - panning) * 255 / 100 : 255;
    ctx->chans[channel].pan_right = (panning < 0) ? (100 + panning) * 255 / 100 : 255;
    unlock_audio(ctx);
}

static void play_music(void *userdata, const music_source *src) {
    dumb_audio_context *ctx = userdata;
    assert(ctx);
    assert(src);
    stop_music(ctx);
    lock_audio(ctx);
    ctx->music = *src;
    music_source_set_volume(&ctx->music, ctx->music_volume);
    unlock_audio(ctx);
}

void dumb_audio_backend_set_callbacks(audio_backend *backend) {
    backend->is_available = is_available;
    backend->get_description = get_description;
    backend->get_name = get_name;
    backend->get_sample_rates = get_sample_rates;
    backend->get_info = get_info;
    backend->create = create_backend;
    backend->destroy = destroy_backend;
    backend->set_music_volume = set_backend_music_volume;
    backend->set_sound_volume = set_backend_sound_volume;
    backend->setup_context = setup_backend_context;
    backend->close_context = close_backend_context;
    backend->play_pcm_sound = play_pcm_sound;
    backend->is_channel_playing = is_channel_playing;
    backend->stop_channel = stop_channel;
    backend->fade_out_channel = fade_out_channel;
    backend->set_channel_panning = set_channel_panning;
    backend->play_music = play_music;
    backend->stop_music = stop_music;
}
