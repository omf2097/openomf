#ifndef _MUSIC_H
#define _MUSIC_H

#include "audio/source.h"

typedef struct {
	int id;
	const char *name;
} module_source;

int music_play(unsigned int id);
/* Equivalent to music_stop() + music_play() */
int music_reload();
void music_stop();
int music_playing();
void music_set_volume(float volume);
unsigned int music_get_resource();
module_source* music_get_module_sources();

#endif // _MUSIC_H
