#include "audio/stream.h"
#include "utils/log.h"
#include <AL/al.h>
#include <AL/alc.h>

int audio_stream_create(audio_stream *stream) {
    // Reserve resources
    alGenSources(1, &stream->alsource);
    alGenBuffers(AUDIO_BUFFER_COUNT, stream->albuffers);
    if(alGetError() != AL_NO_ERROR) {
        alDeleteSources(1, &stream->alsource);
        ERROR("Could not create OpenAL buffers!");
        return 1;
    }
    
    // Pick format
    switch(stream->bytes) {
        case 1: switch(stream->channels) {
            case 1: stream->alformat = AL_FORMAT_MONO8; break;
            case 2: stream->alformat = AL_FORMAT_STEREO8; break;
        }; break;
        case 2: switch(stream->channels) {
            case 1: stream->alformat = AL_FORMAT_MONO16; break;
            case 2: stream->alformat = AL_FORMAT_STEREO16; break;
        }; break;
    };
    if(!stream->alformat) {
        alDeleteSources(1, &stream->alsource);
        alDeleteBuffers(AUDIO_BUFFER_COUNT, stream->albuffers);
        ERROR("Could not find suitable audio format!");
        return 1;
    }
    
    // All done
    return 0;
}

int audio_stream_start(audio_stream *stream) {
    char buf[AUDIO_BUFFER_SIZE];
    int ret;
    for(unsigned int i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        ret = stream->update(stream, buf, AUDIO_BUFFER_SIZE);
        alBufferData(stream->albuffers[i], stream->alformat, buf, ret, stream->frequency);
    }
    alSourceQueueBuffers(stream->alsource, AUDIO_BUFFER_COUNT, stream->albuffers);
    alSourcePlay(stream->alsource);
    return 0;
}


int audio_stream_render(audio_stream *stream) {
    // See if we have any empty buffers to fill
    int val;
    alGetSourcei(stream->alsource, AL_BUFFERS_PROCESSED, &val);
    if(val <= 0) {
        return 0;
    }

    // Handle buffer filling and loading
    char buf[AUDIO_BUFFER_SIZE];
    ALuint bufno;
    int ret = 0;
    while(val--) {
        // Fill buffer & re-queue
        ret = stream->update(stream, buf, AUDIO_BUFFER_SIZE);
        alSourceUnqueueBuffers(stream->alsource, 1, &bufno);
        alBufferData(bufno, stream->alformat, buf, ret, stream->frequency);
        alSourceQueueBuffers(stream->alsource, 1, &bufno);
    }
    return 0;
}

void audio_stream_free(audio_stream *stream) {
    // Free resources
    alDeleteSources(1, &stream->alsource);
    alDeleteBuffers(AUDIO_BUFFER_COUNT, stream->albuffers);
}