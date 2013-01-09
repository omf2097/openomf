#include "audio/audio.h"
#include "utils/log.h"
#include "utils/list.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>

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

}

void audio_close() {
    list_free(&streams);
    alcMakeContextCurrent(0);
    alcDestroyContext(alctx);
    alcCloseDevice(aldevice);
    DEBUG("Audio deinit.");
}