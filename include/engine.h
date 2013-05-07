#ifndef _ENGINE_H
#define _ENGINE_H

typedef struct settings_t settings;
typedef struct scene_t scene;
typedef struct engine_global_t engine_global;

// TODO move this to a different file?
struct engine_global_t {
    scene *scene;
    settings *const settings;
};

int engine_init(); // Init window, audiodevice, etc.
void engine_run(); // Run game
void engine_close(); // Kill window, audiodev

engine_global *engine_globals();

#endif // _ENGINE_H
