#include "game/particle.h"

int particle_load(particle *p, unsigned int id, animation *ani, int x, int y) {
    physics_init(&p->phy, x, y, 0.0f, 0.0f, 190, 10, 24, 295, 1.0f, p);
    return animationplayer_create(&p->player, id, ani);
}

void particle_free(particle *p) {
    animationplayer_free(&p->player);
}

void particle_tick(particle *p) {
    physics_tick(&p->phy);
    animationplayer_run(&p->player);
}

void particle_render(particle *p) {
    animationplayer_render(&p->player);
}
