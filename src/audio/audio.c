#include "audio/audio.h"
#include "audio/stream.h"
#include "utils/log.h"
#include "utils/list.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdlib.h>

ALCdevice *aldevice;
ALCcontext *alctx;
list streams;

int audio_init() {
    // Initialize device
    aldevice = alcOpenDevice(0);
    if(!aldevice) {
        ERROR("Could not open audio playback device!");
        return 1;
    }

    // Create context & make it current
    alctx = alcCreateContext(aldevice, 0);
    alcMakeContextCurrent(alctx);
    
    // List for streams
    list_create(&streams);

    // Some log stuff
    DEBUG("Audio Init OK");
    DEBUG(" * Vendor:      %s", alGetString(AL_VENDOR));
    DEBUG(" * Renderer:    %s", alGetString(AL_RENDERER));
    DEBUG(" * Version:     %s", alGetString(AL_VERSION));
    return 0;
}

void audio_render() {
    list_iterator it;
    list_iter(&streams, &it);
    audio_stream *stream;
    while((stream = list_next(&it)) != 0) {
        if(audio_stream_render(stream)) {
            stream->close(stream);
            audio_stream_free(stream);
            free(stream);
            list_delete(&streams, &it);
        }
    }
}

void audio_play(audio_stream *stream) {
    audio_stream_start(stream);
    list_push_last(&streams, stream);
}

void audio_close() {
    // Free streams
    list_iterator it;
    list_iter(&streams, &it);
    audio_stream *stream;
    while((stream = list_next(&it)) != 0) {
        stream->close(stream);
        audio_stream_free(stream);
        free(stream);
    }
    list_free(&streams);

    // Kill openal
    alcMakeContextCurrent(0);
    alcDestroyContext(alctx);
    alcCloseDevice(aldevice);
    DEBUG("Audio deinit.");
}