#include "game/particle.h"

int particle_load(particle *p, unsigned int id, animation *ani) {
    return animationplayer_create(&p->player, id, ani);
}

void particle_free(particle *p) {
    animationplayer_free(&p->player);
}

void particle_tick(particle *p) {
    animationplayer_run(&p->player);
}

void particle_render(particle *p) {
    animationplayer_render(&p->player);
}

void particle_collision_har(particle *p, har *h) {

}

void particle_collision_scene(particle *p, scene *c) {

}
