#include "audio/sound.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <string.h>
#include <stdlib.h>
#include "utils/log.h"

typedef struct sound_effect_t {
    const char *data;
    unsigned int len;
    unsigned int pos;
} sound_effect;


void audio_stream_pan(audio_stream *stream, int dt) {
    sound_effect *se = (sound_effect*)stream->userdata;
    float t = (stream->snd.pan_end-stream->snd.pan_start)*(dt/(se->len/8.0f));
    stream->snd.pan += t;
    if((t < 0.0f && stream->snd.pan < stream->snd.pan_end) || 
       (t > 0.0f && stream->snd.pan > stream->snd.pan_end)) { 
       stream->snd.pan = stream->snd.pan_end; 
    }
    float pos[] = {stream->snd.pan, 0.0f, 0.0f}; 
    alSourcefv(stream->alsource, AL_POSITION, pos);
}

void sound_preupdate(audio_stream *stream, int dt) {
    audio_stream_pan(stream, dt);
}

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

int sound_play(const char *data, unsigned int len, sound_state *ss) {
    // Audio data struct for userdata
    sound_effect *se = malloc(sizeof(sound_effect));
    se->data = data;
    se->pos = 0;
    se->len = len;
    
    // Create stream
    audio_stream stream;
    stream.frequency = 8000;
    stream.channels = 1;
    stream.bytes = 1;
    stream.type = TYPE_EFFECT;
    stream.userdata = (void*)se;
    stream.preupdate = sound_preupdate;
    stream.update = sound_update;
    stream.close = sound_close;
    stream.snd = *ss;
    
    // Create openal stream
    if(audio_stream_create(&stream)) {
        free(se);
        return 1;
    }
    
    // Play
    audio_play(&stream);
    
    // All done
    return 0;
}

