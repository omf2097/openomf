#include "audio/stream.h"
#include "audio/sound.h"
#include "utils/log.h"
#include <AL/al.h>
#include <AL/alc.h>

int audio_stream_create(audio_stream *stream) {
    // Dump old errors
    int error;
    while((error = alGetError()) != AL_NO_ERROR);

    // Reserve resources
    alGenSources(1, &stream->alsource);
    alGenBuffers(AUDIO_BUFFER_COUNT, stream->albuffers);
    if(alGetError() != AL_NO_ERROR) {
        alDeleteSources(1, &stream->alsource);
        PERROR("Could not create OpenAL buffers!");
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
        PERROR("Could not find suitable audio format!");
        return 1;
    }
    
    // Set playing as 0. If playing = 1 and we lose data from buffers, 
    // this will reset playback & refill buffers.
    stream->playing = 0;
    
    // All done
    return 0;
}

void alplay(audio_stream *stream) {
    if(stream->type == TYPE_EFFECT) {
        // apply sound effects
        float pos[] = {stream->snd.pan, 0.0f, 0.0f}; 
        alSourcefv(stream->alsource, AL_POSITION, pos);
        alSourcef(stream->alsource, AL_GAIN, stream->snd.vol);
        // zero freq is not legal
        if(stream->snd.freq != 0.0f) { alSourcef(stream->alsource, AL_PITCH, stream->snd.freq); }
    }
    alSourcePlay(stream->alsource);
}

int audio_stream_start(audio_stream *stream) {
    char buf[AUDIO_BUFFER_SIZE];
    int ret;
    unsigned int i;
    for(i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        ret = stream->update(stream, buf, AUDIO_BUFFER_SIZE);
        alBufferData(stream->albuffers[i], stream->alformat, buf, ret, stream->frequency);
    }
    alSourceQueueBuffers(stream->alsource, AUDIO_BUFFER_COUNT, stream->albuffers);
    alplay(stream);
    stream->playing = 1;
    return 0;
}

int alplaying(audio_stream *stream) {
    ALenum state;
    alGetSourcei(stream->alsource, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

void audio_stream_stop(audio_stream *stream) {
    stream->playing = 0;
    alSourceStop(stream->alsource);
}

int audio_stream_playing(audio_stream *stream) {
    return stream->playing;
}
void audio_stream_set_volume(audio_stream *stream, float vol) {
    alSourcef(stream->alsource, AL_GAIN, vol);
}

int audio_stream_render(audio_stream *stream, int dt) {
    // Don't do anything unless playback has been started
    if(!stream->playing) {
        return 1;
    }
    
    if(stream->preupdate) { stream->preupdate(stream, dt); }

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
    int first = val;
    while(val--) {
        // Fill buffer & re-queue
        ret = stream->update(stream, buf, AUDIO_BUFFER_SIZE);
        if(ret <= 0 && val == first-1) {
            stream->playing = 0;
            return 1;
        }
        
        alSourceUnqueueBuffers(stream->alsource, 1, &bufno);
        alBufferData(bufno, stream->alformat, buf, ret, stream->frequency);
        alSourceQueueBuffers(stream->alsource, 1, &bufno);
        if(alGetError() != AL_NO_ERROR) {
            PERROR("OpenAL: Error buffering!");
        }
    }
    
    // If stream has stopped because of empty buffers, restart playback.
    if(stream->playing && !alplaying(stream)) {
        alplay(stream);
    }
    
    // All done!
    return 0;
}

void audio_stream_free(audio_stream *stream) {
    // Free resources
    alDeleteSources(1, &stream->alsource);
    alDeleteBuffers(AUDIO_BUFFER_COUNT, stream->albuffers);
}
