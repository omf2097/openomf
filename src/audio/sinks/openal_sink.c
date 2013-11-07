#include <AL/al.h>
#include <AL/alc.h>
#include <stdlib.h>
#include "audio/sinks/openal_sink.h"
#include "audio/sinks/openal_stream.h"
#include "utils/log.h"

typedef struct openal_sink_t {
	ALCdevice *device;
	ALCcontext *context;
} openal_sink;

void openal_sink_close(audio_sink *sink) {
	openal_sink *local = sink_get_userdata(sink);
    alcMakeContextCurrent(0);
    alcDestroyContext(local->context);
    alcCloseDevice(local->device);
    free(local);
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
    DEBUG("OpenAL Audio Sink:");
    DEBUG(" * Vendor:      %s", alGetString(AL_VENDOR));
    DEBUG(" * Renderer:    %s", alGetString(AL_RENDERER));
    DEBUG(" * Version:     %s", alGetString(AL_VERSION));

    // All done
    return 0;
}