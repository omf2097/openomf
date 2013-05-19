#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <shadowdive/shadowdive.h>
#include "game/animation.h"
#include "game/animationplayer.h"
#include "game/physics/physics.h"

/* A few thoughts:
 * - Particle vs. Scene, particle vs. Har collisions should probably be handled in har.c
 * - Particle vs. Particle collisions are probably not necessary
 * - Particle should notify finished state by calling "finished" callback.
 */

typedef struct particle_t particle;

struct particle_t {
    animationplayer player;
    physics_state phy;
    
    // Callbacks
    void *userdata;
    // "finished" is called when particle has finished everything and can be removed
    void (*finished)(particle *p, void *userdata); 
};

int particle_load(particle *p, unsigned int id, animation *ani, int x, int y); 
void particle_free(particle *p);
void particle_tick(particle *p);
void particle_render(particle *p);

#endif // _PARTICLE_H