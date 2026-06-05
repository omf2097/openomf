#include "audio/music_sources/opus_source.h"
#include "utils/log.h"

#ifdef OPUSFILE_FOUND
#include "game/gui/osd/osd.h"
#include "utils/allocator.h"
#include "utils/ringbuffer.h"
#include <SDL.h>
#include <opusfile.h>
#include <stdlib.h>

#define BUF_SIZE 32768
#define BUF_READ 4096
#define RING_SIZE (1024 * 64)
#define MAX_FETCH (RING_SIZE / 2)

typedef struct opus_source {
    ring_buffer buffer;
    OggOpusFile *handle;
    SDL_AudioCVT cvt;
    float volume;
    ogg_int64_t loop_start;
    ogg_int64_t loop_end;
    char temp[BUF_SIZE];
} opus_source;

static long opus_get_tag_value(const OpusTags *tags, const char *tag, const char *alt_tag) {
    const char *value = opus_tags_query(tags, tag, 0);
    if(value == NULL) {
        value = opus_tags_query(tags, alt_tag, 0);
    }
    if(value == NULL) {
        return -1;
    }
    char *end;
    const long result = strtol(value, &end, 10);
    if(end == value) {
        return -1;
    }
    return result;
}

static bool opus_resolve_loop_tags(const OpusTags *tags, ogg_int64_t *a, ogg_int64_t *b) {
    const long start = opus_get_tag_value(tags, "LOOPSTART", "LOOP_START");
    const long end = opus_get_tag_value(tags, "LOOPEND", "LOOP_END");
    const long len = opus_get_tag_value(tags, "LOOPLENGTH", "LOOP_LENGTH");
    if(start < 0 || (end < 0 && len < 0)) {
        return false;
    }

    *a = start;
    *b = end < 0 ? start + len : end;
    return true;
}

static void opus_read_loop_tags(opus_source *context) {
    const OpusTags *tags = op_tags(context->handle, -1);
    if(tags == NULL) {
        return;
    }
    if(opus_resolve_loop_tags(tags, &context->loop_start, &context->loop_end)) {
        log_debug("Opus loop tags: start = %lld, end = %lld", context->loop_start, context->loop_end);
    } else {
        log_debug("No loop tags found for opus file!");
    }
}

static void opus_announce_track(const opus_source *context) {
    const OpusTags *const tags = op_tags(context->handle, -1);
    if(tags == NULL) {
        return;
    }
    const char *const artist = opus_tags_query(tags, "ARTIST", 0);
    const char *const title = opus_tags_query(tags, "TITLE", 0);
    if(artist != NULL && title != NULL) {
        osd_print("playing: %s - %s", artist, title);
    } else if(title != NULL) {
        osd_print("playing: %s", title);
    } else if(artist != NULL) {
        osd_print("playing: %s", artist);
    }
}

static void opus_render(void *userdata, char *stream, const int len) {
    opus_source *const context = userdata;
    if(context) {
        // Opus will return us small buffers of data. Read enough to make sure we can cover the requested
        // length in all cases.
        while(rb_length(&context->buffer) < MAX_FETCH) {
            int read_size = BUF_READ;
            if(context->loop_end > 0) {
                const ogg_int64_t pos = op_pcm_tell(context->handle);
                const ogg_int64_t left = (context->loop_end - pos) * 2;
                if(left <= 0) {
                    log_debug("Loop end reached for opus! Rewinding.");
                    op_pcm_seek(context->handle, context->loop_start);
                    continue;
                }
                if(left < read_size) {
                    read_size = (int)left;
                }
            }
            const int ret = op_read_stereo(context->handle, (opus_int16 *)context->temp, read_size);
            if(ret == 0) {
                op_pcm_seek(context->handle, context->loop_start);
            } else {
                context->cvt.buf = (Uint8 *)context->temp;
                context->cvt.len = ret * 4; // samples * 2 bytes * 2 channels
                SDL_ConvertAudio(&context->cvt);
                rb_write(&context->buffer, context->temp, context->cvt.len_cvt);
            }
        }
        rb_read(&context->buffer, stream, len);
        opus_int16 *samples = (opus_int16 *)stream;
        for(int i = 0; i < len / 2; i++) {
            // Correct volume per signed 16bit sample.
            samples[i] *= context->volume;
        }
    }
}

static void opus_close(void *userdata) {
    opus_source *context = userdata;
    if(context != NULL) {
        op_free(context->handle);
        rb_free(&context->buffer);
        omf_free(context);
    }
}

static void opus_set_volume(void *userdata, const float volume) {
    opus_source *const context = userdata;
    if(context != NULL) {
        context->volume = volume;
    }
}

bool opus_load(music_source *src, const int channels, const int sample_rate, const char *file) {
    opus_source *context = omf_calloc(1, sizeof(opus_source));
    rb_create(&context->buffer, RING_SIZE);
    if((context->handle = op_open_file(file, NULL)) == NULL) {
        log_error("Failed to open opus file");
        goto exit_0;
    }
    context->loop_start = 0; // Default loop point, if no tags are set.
    context->loop_end = 0;
    opus_read_loop_tags(context);
    opus_announce_track(context);
    if(SDL_BuildAudioCVT(&context->cvt, AUDIO_S16, 2, 48000, AUDIO_S16, channels, sample_rate) < 0) {
        log_error("Audio converter creation failed: %s", SDL_GetError());
        goto exit_1;
    }

    src->context = context;
    src->set_volume = opus_set_volume;
    src->render = opus_render;
    src->close = opus_close;
    return true;

exit_1:
    op_free(context->handle);
exit_0:
    omf_free(context);
    rb_free(&context->buffer);
    return false;
}

bool opus_load_memory(music_source *src, int channels, int sample_rate, const unsigned char *buffer, size_t buflen) {
    opus_source *context = omf_calloc(1, sizeof(opus_source));
    rb_create(&context->buffer, RING_SIZE);
    if((context->handle = op_open_memory(buffer, buflen, NULL)) == NULL) {
        log_error("Failed to open opus file");
        goto exit_0;
    }
    context->loop_start = 0; // Default loop point, if no tags are set.
    context->loop_end = 0;
    opus_read_loop_tags(context);
    opus_announce_track(context);
    if(SDL_BuildAudioCVT(&context->cvt, AUDIO_S16, 2, 48000, AUDIO_S16, channels, sample_rate) < 0) {
        log_error("Audio converter creation failed: %s", SDL_GetError());
        goto exit_1;
    }

    src->context = context;
    src->set_volume = opus_set_volume;
    src->render = opus_render;
    src->close = opus_close;
    return true;

exit_1:
    op_free(context->handle);
exit_0:
    rb_free(&context->buffer);
    omf_free(context);
    return false;
}

#else

bool opus_load(music_source *src, const int channels, const int sample_rate, const char *file) {
    log_error("No opusfile support!");
    return false;
}

#endif
