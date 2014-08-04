#ifndef _OPENAL_SINK_STREAM_H
#define _OPENAL_SINK_STREAM_H

#ifdef USE_OPENAL

#include "audio/stream.h"
#include "audio/sink.h"

int openal_stream_init(audio_stream *stream, audio_sink *sink);

#endif // USE_OPENAL

#endif // _OPENAL_SINK_STREAM_H
