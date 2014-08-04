#ifndef _MODPLUG_SOURCE
#define _MODPLUG_SOURCE

#ifdef USE_OGGVORBIS

#include "audio/source.h"

int modplug_source_init(audio_source *src, const char* file, int channels);

#endif // USE_MODPLUG

#endif // _MODPLUG_SOURCE
