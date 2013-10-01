#ifndef _SOUNDLOADER_H
#define _SOUNDLOADER_H

int soundloader_init();
void soundloader_play(unsigned int sound, sound_state *ss);
void soundloader_close();

#endif // _SOUNDLOADER_H
