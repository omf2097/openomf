#ifndef VORBIS_SOURCE_H
#define VORBIS_SOURCE_H

#ifdef USE_OGGVORBIS

#include "audio/source.h"

int vorbis_source_init(audio_source *src, const char *file);

#endif // USE_OGGVORBIS

#endif // VORBIS_SOURCE_H
