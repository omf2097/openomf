#include "game/particle.h"
#include "utils/log.h"
#include "game/physics/space.h"
#include <stdlib.h>

int particle_create(particle *p, unsigned int id, animation *ani, int dir, int px, int py, int vx, int vy, float gravity) {
    p->player.userdata = p;
    p->finished = 0;
    p->id = id;
    p->successor = NULL;
    p->lifetime = PARTICLE_NO_LIFETIME;
    p->lifeticks = 0;
    object_create(&p->pobj, global_space, px, py, vx, vy, 1.0f, 1.0f, 0.4f);
    object_set_gravity(&p->pobj, gravity);
    object_set_group(&p->pobj, 2);
    animationplayer_create(&p->player, id, ani, &p->pobj);
    animationplayer_set_direction(&p->player, dir);
    animationplayer_run(&p->player);
    return 0;
}

void particle_free(particle *p) {
    animationplayer_free(&p->player);
    object_free(&p->pobj);
}

void particle_set_lifetime(particle *p, int lifetime) {
    p->lifetime = lifetime;
}

int particle_successor(particle *p) {
    if (p->successor) {
        DEBUG("playing successor animation");
        int direction = p->player.direction;
        animationplayer_free(&p->player);
        animationplayer_create(&p->player, 0, p->successor, &p->pobj);
        animationplayer_set_direction(&p->player, direction);
        p->player.userdata = p;
        animationplayer_run(&p->player);
        p->successor = NULL;
        return 1;
    }
    return 0;
}

void particle_tick(particle *p) {
    p->lifeticks++;
    animationplayer_run(&p->player);
    
    // Check if animation is finished
    if(p->player.finished) {
        p->finished = 1;
        return;
    }
    
    // Check if lifetime is over
    if(p->lifetime != PARTICLE_NO_LIFETIME && p->lifetime < p->lifeticks) {
        p->finished = 1;
        return;
    }
}

void particle_render(particle *p) {
    animationplayer_render(&p->player);
}
