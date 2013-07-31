#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <shadowdive/shadowdive.h>
#include "game/animation.h"
#include "game/animationplayer.h"
#include "game/physics/object.h"

typedef struct particle_t particle;

#define PARTICLE_NO_LIFETIME 0

struct particle_t {
    animationplayer player;
    int finished;
    unsigned int id;
    animation *successor;
    object pobj;
    int lifetime;
    int lifeticks;
};

int particle_create(particle *p, unsigned int id, animation *ani, int dir, int px, int py, int vx, int vy, float gravity);
void particle_free(particle *p);
void particle_set_lifetime(particle *p, int lifetime);
int particle_successor(particle *p);
void particle_tick(particle *p);
void particle_render(particle *p);

#endif // _PARTICLE_H
