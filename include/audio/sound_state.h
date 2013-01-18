#ifndef _SOUND_STATE_H
#define _SOUND_STATE_H

typedef struct sound_state_t {
    float pan; // -1 to 1
    float pan_start; // -1 to 1
    float pan_end; // -1 to 1
    float freq; // 0.5 to 2
    float vol;  // 0 to 1
} sound_state;

#endif // _SOUND_STATE_H