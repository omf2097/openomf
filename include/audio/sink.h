#ifndef SINK_H
#define SINK_H

#include "utils/hashmap.h"
#include "audio/stream.h"
#include "audio/source.h"

#define VOLUME_DEFAULT 1.0f
#define PANNING_DEFAULT 0.0f
#define PITCH_DEFAULT 1.0f

#define VOLUME_MAX 1.0f
#define PANNING_MAX 1.0f
#define PITCH_MAX 2.0f

#define VOLUME_MIN 0.0f
#define PANNING_MIN -1.0f
#define PITCH_MIN 0.5f

typedef struct audio_sink_t audio_sink;
typedef struct audio_stream_t audio_stream;
typedef struct audio_source_t audio_source;

typedef void (*sink_format_stream_cb)(audio_sink *sink, audio_stream *stream);
typedef void (*sink_close_cb)(audio_sink *sink);

struct audio_sink_t {
    hashmap streams;
    void *userdata;
    sink_close_cb close;
    sink_format_stream_cb format_stream;
};

void sink_init(audio_sink *sink);
void sink_play(audio_sink *sink, audio_source *src, int id, float volume, float panning, float pitch);
void sink_stop(audio_sink *sink, int sid);
void sink_free(audio_sink *sink);
void sink_render(audio_sink *sink);
void sink_format_stream(audio_sink *sink, audio_stream *stream);

int sink_is_playing(audio_sink *sink, int sid);

void sink_set_stream_panning(audio_sink *sink, int sid, float panning);
void sink_set_stream_volume(audio_sink *sink, int sid, float volume);
void sink_set_stream_pitch(audio_sink *sink, int sid, float pitch);

float sink_get_stream_panning(audio_sink *sink, int sid);
float sink_get_stream_volume(audio_sink *sink, int sid);
float sink_get_stream_pitch(audio_sink *sink, int sid);

void sink_set_userdata(audio_sink *sink, void *userdata);
void* sink_get_userdata(audio_sink *sink);
void sink_set_close_cb(audio_sink *sink, sink_close_cb cbfunc);
void sink_set_format_stream_cb(audio_sink *sink, sink_format_stream_cb cbfunc);

#endif // SINK_H
