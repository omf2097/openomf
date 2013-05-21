#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <shadowdive/shadowdive.h>
#include "game/animation.h"
#include "game/animationplayer.h"
#include "game/physics/physics.h"

/* A few thoughts:
 * - Particle vs. Scene, particle vs. Har collisions should probably be handled in har.c
 * - Particle vs. Particle collisions are probably not necessary
 * - Particle should notify finished state by settings finshed flag to 1.
 */

typedef struct particle_t particle;

struct particle_t {
    animationplayer player;
    physics_state phy;
    int finished;
    unsigned int id;
    animation *successor;
};

int particle_create(particle *p, unsigned int id, animation *ani, int x, int y, int direction, float gravity);
int particle_successor(particle *p);
void particle_free(particle *p);
void particle_tick(particle *p);
void particle_render(particle *p);

#endif // _PARTICLE_H
