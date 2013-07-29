#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <shadowdive/shadowdive.h>
#include "game/animation.h"
#include "game/animationplayer.h"
#include "game/object.h"

typedef struct particle_t particle;

struct particle_t {
    animationplayer player;
    int finished;
    unsigned int id;
    animation *successor;
    
    object *pobj;
};

int particle_create(particle *p, unsigned int id, animation *ani, int dir, object pobj);
void particle_free(particle *p);
int particle_successor(particle *p);
void particle_tick(particle *p);
void particle_render(particle *p);

#endif // _PARTICLE_H
