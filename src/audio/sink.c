#include <stdlib.h>
#include "audio/sink.h"
#include "utils/log.h"

void sink_init(audio_sink *sink) {
	sink->userdata = NULL;
	sink->close = NULL;
	sink->format_stream = NULL;

	vector_create(&sink->streams, sizeof(audio_stream*));
}

void sink_format_stream(audio_sink *sink, audio_stream *stream) {
    sink->format_stream(sink, stream);
}

void sink_play(audio_sink *sink, audio_source *src) {
	audio_stream *stream = malloc(sizeof(audio_stream));
	stream_init(stream, sink, src);
	sink_format_stream(sink, stream);
	vector_append(&sink->streams, stream);
	stream_play(stream);
}

void sink_stop(audio_sink *sink, audio_source *src) {
	iterator it;
	vector_iter_begin(&sink->streams, &it);
	audio_stream **stream;
	while((stream = iter_next(&it)) != NULL) {
		if((*stream)->src == src) {
			stream_stop(*stream);
			vector_delete(&sink->streams, &it);
		}
	}
}

void sink_render(audio_sink *sink) {
	iterator it;
	vector_iter_begin(&sink->streams, &it);
	audio_stream **stream;
	while((stream = iter_next(&it)) != NULL) {
		if(stream_get_status(*stream) == STREAM_STATUS_FINISHED) {
			vector_delete(&sink->streams, &it);
			stream_free(*stream);
			free(*stream);
		} else {
            stream_render(*stream);
		}
	}
}

void sink_free(audio_sink *sink) {
	// Free streams
	iterator it;
	vector_iter_begin(&sink->streams, &it);
	audio_stream **stream;
	while((stream = iter_next(&it)) != NULL) {
		stream_stop(*stream);
		stream_free(*stream);
		free(*stream);
	}
	vector_free(&sink->streams);

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