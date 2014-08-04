#ifndef _DUMB_SOURCE_H
#define _DUMB_SOURCE_H

#ifdef USE_DUMB

#include "audio/source.h"

int dumb_source_init(audio_source *src, const char* file, int channels);

#endif // USE_DUMB

#endif // _DUMB_SOURCE_H
