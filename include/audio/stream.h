#ifndef _STREAM_H
#define _STREAM_H

#define AUDIO_BUFFER_COUNT 2
#define AUDIO_BUFFER_SIZE 16384

typedef struct audio_stream_t audio_stream;

typedef struct audio_stream_t {
    unsigned int alsource;
    unsigned int albuffers[AUDIO_BUFFER_COUNT];
    int alformat;
    int frequency;
    int channels;
    int bytes;
    int playing;
    void *userdata;
    int (*update)(audio_stream *stream, char *buf, int len);
    void (*close)(audio_stream *stream);
} audio_stream;

int audio_stream_create(audio_stream *stream);
int audio_stream_start(audio_stream *stream);
int audio_stream_render(audio_stream *stream);
void audio_stream_free(audio_stream *stream);
void audio_stream_stop(audio_stream *stream);
int audio_stream_playing(audio_stream *stream);

#endif // _STREAM_H