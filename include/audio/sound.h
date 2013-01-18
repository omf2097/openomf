#ifndef _SOUND_H
#define _SOUND_H

#include "audio/sound_state.h"

int sound_play(const char *data, unsigned int len, sound_state *ss);

#endif // _SOUND_H