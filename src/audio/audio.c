#include "audio/stream.h"
#include "audio/audio.h"
#include "utils/log.h"
#include "utils/list.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdlib.h>
#include <string.h>

ALCdevice *aldevice;
ALCcontext *alctx;
list streams;

void audio_get_output_list(list *devlist) {
    const ALCchar *device, *devices, *next;
    size_t len;

    if(alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE) { 
        devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
        device = devices;
        next = devices + 1;
        len = 0;

        while(device && *device != '\0' && next && *next != '\n') {
            len = strlen(device);
            list_append(devlist, device, len+1);
            device += (len + 1);
            next += (len + 2);
        }
    }
}

void audio_debug_print_output_devs() {
    list dev_list;
    iterator it;
    char *tmp;
    list_create(&dev_list);
    audio_get_output_list(&dev_list);
    list_iter_begin(&dev_list, &it);
    DEBUG("Audio output devices:");
    while((tmp = iter_next(&it)) != NULL) {
        DEBUG(" * %s", tmp);
    }
    list_free(&dev_list);
}

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
    
    audio_debug_print_output_devs();
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

void audio_set_volume(int type, float vol) {
    iterator it;
    list_iter_begin(&streams, &it);
    audio_stream *stream;
    while((stream = iter_next(&it)) != NULL) {
        if(type == stream->type) {
            audio_stream_set_volume(stream, vol);
        }
    }
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
