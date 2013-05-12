#ifndef _STREAM_H
#define _STREAM_H

#include "audio/sound_state.h"

#define AUDIO_BUFFER_COUNT 3
#define AUDIO_BUFFER_SIZE 32768

typedef struct audio_stream_t audio_stream;

enum {
    TYPE_MUSIC,
    TYPE_EFFECT
};

struct audio_stream_t {
    unsigned int alsource;
    unsigned int albuffers[AUDIO_BUFFER_COUNT];
    int alformat;
    int frequency;
    int channels;
    int bytes;
    int type;
    int playing;
    sound_state snd;
    void *userdata;
    void (*preupdate)(audio_stream *stream, int dt);
    int (*update)(audio_stream *stream, char *buf, int len);
    void (*close)(audio_stream *stream);
};

int audio_stream_create(audio_stream *stream);
int audio_stream_start(audio_stream *stream);
int audio_stream_render(audio_stream *stream, int dt);
void audio_stream_free(audio_stream *stream);
void audio_stream_stop(audio_stream *stream);
int audio_stream_playing(audio_stream *stream);
void audio_stream_set_volume(audio_stream *stream, float vol);

#endif // _STREAM_H
