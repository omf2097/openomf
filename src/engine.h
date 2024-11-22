#ifndef ENGINE_H
#define ENGINE_H

typedef struct engine_init_flags_t {
    unsigned int net_mode;
    unsigned int record;
    char rec_file[255];
} engine_init_flags;

int engine_init(engine_init_flags *init_flags); // Init window, audiodevice, etc.
void engine_run(engine_init_flags *init_flags); // Run game
void engine_close(void);                        // Kill window, audiodev

#endif // ENGINE_H
