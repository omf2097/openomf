#ifndef AP_ARENA_H
#define AP_ARENA_H

#include "game/protos/scene.h"

void ap_arena_attach(scene *s);
void ap_arena_detach(void);
void ap_arena_tick(void);
void ap_arena_render(void);

#endif // AP_ARENA_H
