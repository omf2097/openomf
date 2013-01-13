#include "audio/stream.h"
#include "audio/audio.h"
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
        PERROR("Could not open audio playback device!");
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

void audio_render(int dt) {
    iterator it;
    list_iter_begin(&streams, &it);
    audio_stream *stream;
    while((stream = iter_next(&it)) != NULL) {
        if(audio_stream_render(stream, dt)) {
            audio_stream_stop(stream);
            stream->close(stream);
            audio_stream_free(stream);
            list_delete(&streams, &it);
        }
    }
}

audio_stream* audio_get_music() {
    iterator it;
    list_iter_begin(&streams, &it);
    audio_stream *stream;
    while((stream = iter_next(&it)) != NULL) {
        if(stream->type == TYPE_MUSIC) {
            return stream;
        }
    }
    return NULL;
}

void audio_play(audio_stream *stream) {
    audio_stream_start(stream);
    list_append(&streams, stream, sizeof(audio_stream));
}

void audio_close() {
    // Free streams
    iterator it;
    list_iter_begin(&streams, &it);
    audio_stream *stream;
    while((stream = iter_next(&it)) != NULL) {
        audio_stream_stop(stream);
        stream->close(stream);
        audio_stream_free(stream);
    }
    list_free(&streams);

    // Kill openal
    alcMakeContextCurrent(0);
    alcDestroyContext(alctx);
    alcCloseDevice(aldevice);
    DEBUG("Audio deinit.");
}
