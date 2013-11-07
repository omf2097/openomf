#ifndef _VORBIS_SOURCE_H
#define _VORBIS_SOURCE_H

#ifdef USE_OGGVORBIS

#include "audio/source.h"

int vorbis_source_init(audio_source *src, const char* file);

#endif // USE_OGGVORBIS

#endif // _VORBIS_SOURCE_H