#ifndef STREAM_H
#define STREAM_H

typedef struct audio_stream_t audio_stream;
typedef struct audio_source_t audio_source;
typedef struct audio_sink_t audio_sink;

typedef void (*stream_play_cb)(audio_stream *stream);
typedef void (*stream_stop_cb)(audio_stream *stream);
typedef void (*stream_update_cb)(audio_stream *stream);
typedef void (*stream_apply_cb)(audio_stream *stream);
typedef void (*stream_close_cb)(audio_stream *stream);

enum {
    STREAM_STATUS_PLAYING,
    STREAM_STATUS_STOPPED,
    STREAM_STATUS_FINISHED
};

struct audio_stream_t {
    int status;
    float panning;
    float volume;
    float pitch;

    void *userdata;
    stream_update_cb update;
    stream_close_cb close;
    stream_play_cb play;
    stream_stop_cb stop;
    stream_apply_cb apply;

    audio_source *src;
    audio_sink *sink;
};

void stream_init(audio_stream *stream, audio_sink *sink, audio_source *src);
void stream_render(audio_stream *stream);
void stream_free(audio_stream *stream);
void stream_play(audio_stream *stream);
void stream_stop(audio_stream *stream);
void stream_apply(audio_stream *stream);
void stream_set_finished(audio_stream *stream);
int stream_get_status(audio_stream *stream);

void stream_set_userdata(audio_stream *stream, void *userdata);
void* stream_get_userdata(audio_stream *stream);
void stream_set_update_cb(audio_stream *stream, stream_update_cb cbfunc);
void stream_set_close_cb(audio_stream *stream, stream_close_cb cbfunc);
void stream_set_play_cb(audio_stream *stream, stream_play_cb cbfunc);
void stream_set_stop_cb(audio_stream *stream, stream_stop_cb cbfunc);
void stream_set_apply_cb(audio_stream *stream, stream_apply_cb cbfunc);

#endif // STREAM_H
