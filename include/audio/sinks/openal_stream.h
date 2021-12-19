#ifndef OPENAL_STREAM_H
#define OPENAL_STREAM_H

#ifdef USE_OPENAL

#include "audio/stream.h"
#include "audio/sink.h"

int openal_stream_init(audio_stream *stream, audio_sink *sink);

#endif // USE_OPENAL

#endif // OPENAL_STREAM_H
