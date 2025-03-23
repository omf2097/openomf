#include "audio/sources/opus_source.h"
#include "utils/log.h"

#ifdef OPUSFILE_FOUND
#include "utils/allocator.h"
#include "utils/ringbuffer.h"
#include <assert.h>
#include <opusfile.h>

#define BUF_SIZE 8192
#define RING_SIZE (1024 * 64)
#define MAX_FETCH (RING_SIZE / 2)

typedef struct opus_source {
    ring_buffer buffer;
    OggOpusFile *handle;
    float volume;
} opus_source;

static void opus_render(void *userdata, char *stream, int len) {
    opus_source *context = userdata;
    if(context) {
        int ret;
        opus_int16 temp[BUF_SIZE];
        // Opus will return us small buffers of data. Read enough to make sure we can cover the requested
        // length in all cases.
        while(rb_length(&context->buffer) < MAX_FETCH) {
            ret = op_read_stereo(context->handle, temp, BUF_SIZE);
            if(ret == 0) {
                // Stream ended, seek back to 0 offset and begin again.
                op_raw_seek(context->handle, 0);
            } else {
                rb_write(&context->buffer, (char *)temp, ret * 4);
            }
        }
        rb_read(&context->buffer, stream, len);
        for(int i = 0; i < len; i++) {
            stream[i] *= context->volume;
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

static void opus_set_volume(void *userdata, float volume) {
    opus_source *context = userdata;
    if(context != NULL) {
        context->volume = volume;
    }
}

bool opus_load(music_source *src, int channels, int sample_rate, const char *file) {
    // Note: Opus decoder only supports 48kHz / stereo.
    assert(sample_rate == 48000);
    assert(channels == 2);

    opus_source *context = omf_calloc(1, sizeof(opus_source));
    rb_create(&context->buffer, RING_SIZE);
    if((context->handle = op_open_file(file, NULL)) == NULL) {
        log_error("Failed to open opus file");
        goto exit_0;
    }

    src->context = context;
    src->set_volume = opus_set_volume;
    src->render = opus_render;
    src->close = opus_close;
    return true;

exit_0:
    omf_free(context);
    return false;
}

#else

bool opus_load(music_source *src, int channels, int sample_rate, const char *file) {
    // Just fail if there is no opus support.
    log_error("No opusfile support!");
    return false;
}

#endif
