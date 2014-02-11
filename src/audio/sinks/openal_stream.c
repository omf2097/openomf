// On Apple OS (Mac and IOS), al.h and alc.h are in nonstandard locations
#ifdef __APPLE__
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <stdlib.h>
#include "audio/sinks/openal_stream.h"
#include "utils/log.h"

#define AUDIO_BUFFER_COUNT 2
#define AUDIO_BUFFER_SIZE 32768

typedef struct openal_stream_t {
    unsigned int source;
    unsigned int buffers[AUDIO_BUFFER_COUNT];
    int format;
} openal_stream;

void openal_stream_apply(audio_stream *stream) {
    openal_stream *local = stream_get_userdata(stream);

    // Handle panning here (only if mono)
    if(source_get_channels(stream->src) == 1) {
        float pos[] = {stream->panning, 0.0f, 0.0f};
        alSourcefv(local->source, AL_POSITION, pos);
    }

    // Volume and pitch
    alSourcef(local->source, AL_GAIN, stream->volume);
    alSourcef(local->source, AL_PITCH, stream->pitch);
}

void openal_stream_play(audio_stream *stream) {
    openal_stream *local = stream_get_userdata(stream);

    // Fill initial buffers
    char buf[AUDIO_BUFFER_SIZE];
    for(int i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        int ret = source_update(stream->src, buf, AUDIO_BUFFER_SIZE);
        if(ret > 0) { 
            alBufferData(
                local->buffers[i], 
                local->format, 
                buf, ret, 
                source_get_frequency(stream->src));
            alSourceQueueBuffers(local->source, 1, &local->buffers[i]);
        }
    }

    // Set volume etc.
    openal_stream_apply(stream);

    // Start playback
    alSourcePlay(local->source);
    int err = alGetError();
    if(err != AL_NO_ERROR) {
        PERROR("OpenAL Stream: Source playback error: %d.", err);
    }
}

void openal_stream_stop(audio_stream *stream) {
    openal_stream *local = stream_get_userdata(stream);
    alSourceStop(local->source);
}

void openal_stream_update(audio_stream *stream) {
    openal_stream *local = stream_get_userdata(stream);

    // See if we have any empty buffers to fill
    int val;
    alGetSourcei(local->source, AL_BUFFERS_PROCESSED, &val);
    if(val <= 0) {
        return;
    }

    // Handle buffer filling and loading
    char buf[AUDIO_BUFFER_SIZE];
    ALuint n;
    while(val--) {
        // Fill buffer & re-queue
        int ret = source_update(stream->src, buf, AUDIO_BUFFER_SIZE);
        if(ret > 0) {
            alSourceUnqueueBuffers(local->source, 1, &n);
            alBufferData(n, local->format, buf, ret, source_get_frequency(stream->src));
            alSourceQueueBuffers(local->source, 1, &n);

            // Check for any errors
            int err = alGetError();
            if(err != AL_NO_ERROR) {
                PERROR("OpenAL Stream: Error %d while buffering!", err);
            }
        } else {
            stream_set_finished(stream);
            break;
        }
    }

    // Make sure we are playing stream
    if(stream_get_status(stream) == STREAM_STATUS_PLAYING) {
        ALenum state;
        alGetSourcei(local->source, AL_SOURCE_STATE, &state);
        if(state != AL_PLAYING) {
            alSourcePlay(local->source);
        }
    }
}

void openal_stream_close(audio_stream *stream) {
    openal_stream *local = stream_get_userdata(stream);
    alSourceStop(local->source);
    alDeleteSources(1, &local->source);
    alDeleteBuffers(AUDIO_BUFFER_COUNT, local->buffers);
    free(local);
}

int openal_stream_init(audio_stream *stream, audio_sink *sink) {
    openal_stream *local = malloc(sizeof(openal_stream));

    // Dump old errors
    int error;
    while((error = alGetError()) != AL_NO_ERROR);

    // Pick format
    local->format = 0;
    switch(source_get_bytes(stream->src)) {
        case 1: switch(source_get_channels(stream->src)) {
            case 1: local->format = AL_FORMAT_MONO8; break;
            case 2: local->format = AL_FORMAT_STEREO8; break;
        }; break;
        case 2: switch(source_get_channels(stream->src)) {
            case 1: local->format = AL_FORMAT_MONO16; break;
            case 2: local->format = AL_FORMAT_STEREO16; break;
        }; break;
    };
    if(!local->format) {
        PERROR("OpenAL Stream: Could not find suitable audio format!");
        goto exit_0;
    }
    
    // Generate a source
    alGenSources(1, &local->source);
    if(alGetError() != AL_NO_ERROR) {
        PERROR("OpenAL Stream: Could not create audio source!");
        goto exit_0;
    }
    
    // Generate buffers
    alGenBuffers(AUDIO_BUFFER_COUNT, local->buffers);
    if(alGetError() != AL_NO_ERROR) {
        PERROR("OpenAL Stream: Could not create audio buffers!");
        goto exit_1;
    }

    // Set callbacks etc.
    stream_set_userdata(stream, local);
    stream_set_update_cb(stream, openal_stream_update);
    stream_set_close_cb(stream, openal_stream_close);
    stream_set_play_cb(stream, openal_stream_play);
    stream_set_stop_cb(stream, openal_stream_stop);
    stream_set_apply_cb(stream, openal_stream_apply);
    
    // All done
    return 0;

    // Error exits
exit_1:
    alDeleteSources(1, &local->source);
exit_0:
    free(local);
    return 1;
}
