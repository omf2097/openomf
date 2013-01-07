#ifndef _ENGINE_H
#define _ENGINE_H

#include "audio/audio.h"
#include "video/video.h"

int engine_init(); // Init window, audiodevice, etc.
void engine_run(); // Run game
void engine_close(); // Kill window, audiodev

#endif // _ENGINE_H