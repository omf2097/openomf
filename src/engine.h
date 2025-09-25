#ifndef ENGINE_H
#define ENGINE_H

#include "utils/path.h"

// static tick duration, in ms
#define STATIC_TICKS 10

typedef struct engine_init_flags {
    unsigned int net_mode;
    unsigned int record;
    unsigned int playback;
    char force_renderer[16];
    char force_audio_backend[16];
    path *rec_files;
    int warpspeed;
    int speed;
} engine_init_flags;

int engine_init(engine_init_flags *init_flags); // Init window, audiodevice, etc.
void engine_run(engine_init_flags *init_flags); // Run game
void engine_close(void);                        // Kill window, audiodev

#endif // ENGINE_H
