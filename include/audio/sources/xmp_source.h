#ifndef _XMP_SOURCE
#define _XMP_SOURCE

#ifdef USE_XMP

#include "audio/source.h"

int xmp_source_init(audio_source *src, const char* file, int channels);

#endif // USE_XMP

#endif // _XMP_SOURCE
