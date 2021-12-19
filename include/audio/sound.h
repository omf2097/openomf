#ifndef SOUND_H
#define SOUND_H

void sound_play(int id, float volume, float panning, float pitch);
int sound_playing(unsigned int sound_id);
void sound_set_volume(float volume);

#endif // SOUND_H
