#include <stdlib.h>
#include "audio/sink.h"
#include "utils/log.h"

unsigned int _sink_global_id = 1;

unsigned int gid_gen() {
	return _sink_global_id++;
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
	audio_stream *stream = malloc(sizeof(audio_stream));
	stream_init(stream, sink, src);
	sink_format_stream(sink, stream);
	stream_play(stream);
	unsigned int new_key = gid_gen();
	hashmap_iput(&sink->streams, new_key, &stream, sizeof(audio_stream*));
	return new_key;
}

void sink_stop(audio_sink *sink, unsigned int sid) {
	// Find stream
	audio_stream **s;
	unsigned int len;
	if(hashmap_iget(&sink->streams, sid, (void**)&s, &len) != 0) {
		return; // Key not found
	}

	// Stop playback && remove stream
	stream_stop(*s);
	stream_free(*s);
	free(*s);
	hashmap_idel(&sink->streams, sid);
}

void sink_render(audio_sink *sink) {
	iterator it;
	hashmap_iter_begin(&sink->streams, &it);
	audio_stream *stream;
	unsigned int key;
	hashmap_pair *pair;
	while((pair = iter_next(&it)) != NULL) {
		stream = *((audio_stream**)pair->val);
		if(stream_get_status(stream) == STREAM_STATUS_FINISHED) {
			key = *(unsigned int *)pair->key;
			stream_free(stream);
			free(stream);
			hashmap_idel(&sink->streams, key);
		} else {
            stream_render(stream);
		}
	}
}

void sink_free(audio_sink *sink) {
	// Free streams
	iterator it;
	hashmap_iter_begin(&sink->streams, &it);
	audio_stream *stream;
	hashmap_pair *pair;
	while((pair = iter_next(&it)) != NULL) {
		stream = pair->val;
		stream_stop(stream);
		stream_free(stream);
	}
	hashmap_free(&sink->streams);

	// Close sink
	if(sink->close != NULL) {
		sink->close(sink);
	}
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