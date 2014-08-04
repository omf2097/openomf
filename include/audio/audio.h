#ifndef _AUDIO_H
#define _AUDIO_H

#include "audio/music.h"
#include "audio/sound.h"
#include "audio/sink.h"
#include "audio/source.h"
#include "audio/stream.h"
#include "audio/sources/dumb_source.h"
#include "audio/sources/vorbis_source.h"

int audio_get_sink_count();
const char* audio_get_sink_name(int id);
int audio_is_sink_available(const char* sink_name);
const char* audio_get_first_sink_name();
int audio_init(const char* sink_name);
void audio_render();
void audio_close();

audio_sink* audio_get_sink();

#endif // _AUDIO_H
