#ifdef USE_OPENAL

// On Apple OS (Mac and IOS), al.h and alc.h are in nonstandard locations
#ifdef __APPLE__
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <stdlib.h>
#include "audio/sinks/openal_sink.h"
#include "audio/sinks/openal_stream.h"
#include "utils/log.h"

typedef struct {
    ALCdevice *device;
    ALCcontext *context;
} openal_sink;

void openal_sink_close(audio_sink *sink) {
    openal_sink *local = sink_get_userdata(sink);
    alcMakeContextCurrent(0);
    alcDestroyContext(local->context);
    alcCloseDevice(local->device);
    free(local);
    INFO("OpenAL Sink closed.");
}

void openal_sink_format_stream(audio_sink *sink, audio_stream *stream) {
    openal_stream_init(stream, sink);
}

int openal_sink_init(audio_sink *sink) {
    openal_sink *local = malloc(sizeof(openal_sink));

    // Open device and create context
    local->device = alcOpenDevice(0);
    if(!local->device) {
        PERROR("Could not open audio playback device!");
        free(local);
        return 1;
    }
    local->context = alcCreateContext(local->device, 0);
    alcMakeContextCurrent(local->context);

    // Set callbacks
    sink_set_userdata(sink, local);
    sink_set_close_cb(sink, openal_sink_close);
    sink_set_format_stream_cb(sink, openal_sink_format_stream);

    // Some log stuff
    INFO("OpenAL Audio Sink:");
    INFO(" * Vendor:      %s", alGetString(AL_VENDOR));
    INFO(" * Renderer:    %s", alGetString(AL_RENDERER));
    INFO(" * Version:     %s", alGetString(AL_VERSION));

    // All done
    return 0;
}

#endif // USE_OPENAL
