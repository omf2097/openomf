#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <shadowdive/shadowdive.h>
#include "game/animation.h"
#include "game/animationplayer.h"
#include "game/physics/physics.h"
#include "game/har.h"
#include "game/scene.h"

typedef struct particle_t particle;

struct particle_t {
    animationplayer player;
    physics_state phy;
    
    // Callbacks
    void *userdata;
    // "finished" is called when particle has finished everything and can be removed
    void (*finished)(particle *p, void *userdata); 
};

int particle_load(particle *p, unsigned int id, animation *ani); 
void particle_free(particle *p);
void particle_tick(particle *p);
void particle_render(particle *p);
void particle_collision_har(particle *p, har *h);
void particle_collision_scene(particle *p, scene *c);

#endif // _PARTICLE_H