#ifndef _MUSIC_H
#define _MUSIC_H

#include "audio/source.h"

int music_play(const char *filename);
void music_stop();
int music_playing();
void music_set_volume(float volume);

#endif // _MUSIC_H
