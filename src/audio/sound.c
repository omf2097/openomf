#include "audio/sound.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <string.h>
#include <stdlib.h>

typedef struct sound_effect_t {
    const char *data;
    unsigned int len;
    unsigned int pos;
} sound_effect;

int sound_update(audio_stream *stream, char *buf, int len) {
    sound_effect *se = (sound_effect*)stream->userdata;
    int left = se->len - se->pos;
    int copy = (left > len) ? len : left;
    if(copy > 0) {
        memcpy(buf, se->data, copy);
    }
    se->pos += copy;
    return copy;
}

void sound_close(audio_stream *stream) {
    sound_effect *se = (sound_effect*)stream->userdata;
    free(se);
}

int sound_play(const char *data, unsigned int len) {
    // Audio data struct for userdata
    sound_effect *se = malloc(sizeof(sound_effect));
    se->data = data;
    se->pos = 0;
    se->len = len;
    
    // Create stream
    audio_stream *stream = malloc(sizeof(audio_stream));
    stream->frequency = 8000;
    stream->channels = 1;
    stream->bytes = 1;
    stream->userdata = (void*)se;
    stream->update = &sound_update;
    stream->close = &sound_close;
    
    // Create openal stream
    if(audio_stream_create(stream)) {
        free(stream);
        free(se);
        return 1;
    }
    
    // Play
    audio_play(stream);
    
    // All done
    return 0;
}

