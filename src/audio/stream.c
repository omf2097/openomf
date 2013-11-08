#include <stdlib.h>
#include "audio/stream.h"
#include "audio/source.h"
#include "utils/log.h"

void stream_init(audio_stream *stream, audio_sink *sink, audio_source *src) {
	stream->userdata = NULL;
	stream->update = NULL;
	stream->close = NULL;
    stream->play = NULL;
    stream->stop = NULL;
	stream->status = STREAM_STATUS_STOPPED;
	stream->src = src;
    stream->sink = sink;
}

void stream_set_finished(audio_stream *stream) {
	stream->status = STREAM_STATUS_FINISHED;
}

void stream_render(audio_stream *stream) {
	if(stream->update != NULL && stream->status == STREAM_STATUS_PLAYING) {
		stream->update(stream);
	}
}

void stream_free(audio_stream *stream) {
	if(stream->close != NULL) {
		stream->close(stream);
		source_free(stream->src);
		free(stream->src);
	}
}

void stream_play(audio_stream *stream) {
	if(stream->play != NULL && stream->status == STREAM_STATUS_STOPPED) {
		stream->play(stream);
		stream->status = STREAM_STATUS_PLAYING;
	}
}

void stream_stop(audio_stream *stream) {
	if(stream->stop != NULL && stream->status == STREAM_STATUS_PLAYING) {
		stream->stop(stream);
		stream->status = STREAM_STATUS_STOPPED;
	}
}

int stream_get_status(audio_stream *stream) {
	return stream->status;
}

void stream_set_userdata(audio_stream *stream, void *userdata) {
	stream->userdata = userdata;
}

void* stream_get_userdata(audio_stream *stream) {
	return stream->userdata;
}

void stream_set_update_cb(audio_stream *stream, stream_update_cb cbfunc) {
	stream->update = cbfunc;
}

void stream_set_close_cb(audio_stream *stream, stream_close_cb cbfunc) {
	stream->close = cbfunc;
}

void stream_set_play_cb(audio_stream *stream, stream_play_cb cbfunc) {
	stream->play = cbfunc;
}

void stream_set_stop_cb(audio_stream *stream, stream_stop_cb cbfunc) {
	stream->stop = cbfunc;
}
