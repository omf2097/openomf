#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <shadowdive/shadowdive.h>
#include <chipmunk/chipmunk.h>
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
    int finished;
    unsigned int id;
    animation *successor;
    
    cpSpace *space;
    cpShape *line_floor;
    cpShape *line_wall_left;
    cpShape *line_wall_right;
    cpShape *line_ceiling;
    cpBody *obody;
    cpShape *oshape;
};

int particle_create(particle *p, unsigned int id, animation *ani, int dir, int x, int y, int vx, int vy, float mass, float gravity, float friction, float elasticity);
int particle_successor(particle *p);
void particle_get_vel(particle *p, int *vx, int *vy);
void particle_get_pos(particle *p, int *x, int *y);
void particle_free(particle *p);
void particle_tick(particle *p);
void particle_render(particle *p);

#endif // _PARTICLE_H
