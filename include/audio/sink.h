#ifndef _SINK_H
#define _SINK_H

#include "utils/hashmap.h"
#include "audio/stream.h"
#include "audio/source.h"

typedef struct audio_sink_t audio_sink;

typedef void (*sink_format_stream_cb)(audio_sink *sink, audio_stream *stream);
typedef void (*sink_close_cb)(audio_sink *sink);

typedef struct audio_sink_t {
    hashmap streams;
    void *userdata;
    sink_close_cb close;
    sink_format_stream_cb format_stream;
} audio_sink;

void sink_init(audio_sink *sink);
unsigned int sink_play(audio_sink *sink, audio_source *src);
void sink_stop(audio_sink *sink, unsigned int sid);
void sink_free(audio_sink *sink);
void sink_render(audio_sink *sink);
void sink_format_stream(audio_sink *sink, audio_stream *stream);

void sink_set_userdata(audio_sink *sink, void *userdata);
void* sink_get_userdata(audio_sink *sink);
void sink_set_close_cb(audio_sink *sink, sink_close_cb cbfunc);
void sink_set_format_stream_cb(audio_sink *sink, sink_format_stream_cb cbfunc);

#endif // _SINK_H