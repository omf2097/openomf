#ifndef _SOUND_H
#define _SOUND_H

void sound_play(int id, float volume, float panning, float pitch);
int sound_playing(unsigned int sound_id);
void sound_set_volume(float volume);

#endif // _SOUND_H
