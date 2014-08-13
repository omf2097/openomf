#include <stdlib.h>
#include "audio/sink.h"
#include "utils/log.h"

unsigned int _sink_global_id = 1;

unsigned int gid_gen() {
    return _sink_global_id++;
}

audio_stream* sink_get_stream(audio_sink *sink, unsigned int sid) {
    if(sid == 0) return NULL;
    if(sink == NULL) return NULL;
    audio_stream **s;
    unsigned int len;
    if(hashmap_iget(&sink->streams, sid, (void**)&s, &len) != 0) {
    	return NULL;
    }
    return *s;
}

void sink_init(audio_sink *sink) {
    sink->userdata = NULL;
    sink->close = NULL;
    sink->format_stream = NULL;
    hashmap_create(&sink->streams, 6);
}

void sink_format_stream(audio_sink *sink, audio_stream *stream) {
    sink->format_stream(sink, stream);
}

unsigned int sink_play(audio_sink *sink, audio_source *src) {
	return sink_play_set(sink, src, VOLUME_DEFAULT, PANNING_DEFAULT, PITCH_DEFAULT);
}


int sink_is_playing(audio_sink *sink, unsigned int sid) {
    if(sink_get_stream(sink, sid) != NULL) {
        return 1;
    }
    return 0;
}

unsigned int sink_play_set(audio_sink *sink,
						   audio_source *src,
						   float volume,
						   float panning,
						   float pitch) {
    audio_stream *stream = malloc(sizeof(audio_stream));
    stream_init(stream, sink, src);
    sink_format_stream(sink, stream);
    stream->volume = volume;
    stream->panning = panning;
    stream->pitch = pitch;
    stream_play(stream);
    unsigned int new_key = gid_gen();
    hashmap_iput(&sink->streams, new_key, &stream, sizeof(audio_stream*));
    return new_key;
}

void sink_stop(audio_sink *sink, unsigned int sid) {
    // Stop playback && remove stream
    audio_stream *s = sink_get_stream(sink, sid);
    stream_stop(s);
    stream_free(s);
    free(s);
    hashmap_idel(&sink->streams, sid);
}

void sink_render(audio_sink *sink) {
    iterator it;
    hashmap_iter_begin(&sink->streams, &it);
    hashmap_pair *pair;
    while((pair = iter_next(&it)) != NULL) {
        audio_stream *stream = *((audio_stream**)pair->val);
        stream_render(stream);

        // If stream is done, free it here.
        if(stream_get_status(stream) == STREAM_STATUS_FINISHED) {
            stream_stop(stream);
            stream_free(stream);
            free(stream);
            hashmap_delete(&sink->streams, &it);
        }
    }
}

void sink_free(audio_sink *sink) {
    // Free streams
    iterator it;
    hashmap_iter_begin(&sink->streams, &it);
    hashmap_pair *pair;
    while((pair = iter_next(&it)) != NULL) {
        audio_stream *stream = *((audio_stream**)pair->val);
        stream_stop(stream);
        stream_free(stream);
        free(stream);
    }
    hashmap_free(&sink->streams);

    // Close sink
    if(sink->close != NULL) {
        sink->close(sink);
    }
}

void sink_set_stream_panning(audio_sink *sink, unsigned int sid, float panning) {
	if(panning < PANNING_MIN || panning > PANNING_MAX) return;
	audio_stream *s = sink_get_stream(sink, sid);
	s->panning = panning;
	stream_apply(s);
}

void sink_set_stream_volume(audio_sink *sink, unsigned int sid, float volume) {
	if(volume < VOLUME_MIN || volume > VOLUME_MAX) return;
	audio_stream *s = sink_get_stream(sink, sid);
	s->volume = volume;
	stream_apply(s);
}

void sink_set_stream_pitch(audio_sink *sink, unsigned int sid, float pitch) {
	if(pitch < PITCH_MIN || pitch > PITCH_MAX) return;
	audio_stream *s = sink_get_stream(sink, sid);
	s->pitch = pitch;
	stream_apply(s);
}

float sink_get_stream_panning(audio_sink *sink, unsigned int sid) {
	return sink_get_stream(sink, sid)->panning;
}

float sink_get_stream_volume(audio_sink *sink, unsigned int sid) {
	return sink_get_stream(sink, sid)->volume;
}

float sink_get_stream_pitch(audio_sink *sink, unsigned int sid) {
	return sink_get_stream(sink, sid)->pitch;
}

void sink_set_userdata(audio_sink *sink, void *userdata) {
    sink->userdata = userdata;
}

void* sink_get_userdata(audio_sink *sink) {
    return sink->userdata;
}

void sink_set_close_cb(audio_sink *sink, sink_close_cb cbfunc) {
    sink->close = cbfunc;
}

void sink_set_format_stream_cb(audio_sink *sink, sink_format_stream_cb cbfunc) {
    sink->format_stream = cbfunc;
}
