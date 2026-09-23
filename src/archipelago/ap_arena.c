#include "ap_arena.h"

#include "archipelago/apconnect.h"
#include "utils/log.h"

static scene *g_arena_scene = NULL;

void ap_arena_attach(scene *s) {
    g_arena_scene = s;
}

void ap_arena_detach(void) {
    g_arena_scene = NULL;
}

void ap_arena_tick(void) {
}

void ap_arena_render(void) {
}
