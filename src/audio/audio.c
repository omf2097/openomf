#include "audio/audio.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>

ALCdevice *aldevice;
ALCcontext *alctx;

int audio_init() {
    // Initialize device
    aldevice = alcOpenDevice(0);
    if(!aldevice) {
        printf("[E] Could not open audio playback device!\n");
        return 1;
    }

    // Create context & make it current
    alctx = alcCreateContext(aldevice, 0);
    alcMakeContextCurrent(alctx);

    // Some log stuff
    printf("[D] Audio Init OK\n");
    printf("[D] * Vendor:      %s\n", alGetString(AL_VENDOR));
    printf("[D] * Renderer:    %s\n", alGetString(AL_RENDERER));
    printf("[D] * Version:     %s\n", alGetString(AL_VERSION));
    return 0;
}

void audio_render() {

}

void audio_close() {
    alcMakeContextCurrent(0);
    alcDestroyContext(alctx);
    alcCloseDevice(aldevice);
    printf("[D] Audio deinit.\n");
}