#ifndef _MUSIC_H
#define _MUSIC_H

#include "audio/source.h"

int music_play(unsigned int id);
void music_stop();
int music_playing();
void music_set_volume(float volume);
unsigned int music_get_resource();

#endif // _MUSIC_H
