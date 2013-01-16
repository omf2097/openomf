#ifndef _SOUND_H
#define _SOUND_H

typedef struct sound_state_t {
    float pan; // -1 to 1
    float pan_start; // -1 to 1
    float pan_end; // -1 to 1
    float freq; // 0.5 to 2
    float vol;  // 0 to 1
} sound_state;

int sound_play(const char *data, unsigned int len, sound_state *ss);

#endif // _SOUND_H