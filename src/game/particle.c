#include "game/particle.h"

#include "utils/log.h"

/*
void particle_phys(void *userdata, int x, int y) {
    particle *particle = userdata;
    DEBUG("setting particle velocity %d, %d", x, y);
    physics_recoil(&particle->phy, (float)x, (float)y);
    physics_tick(&particle->phy);
}

void particle_pos(void *userdata, int x, int y) {
    particle *particle = userdata;
    DEBUG("setting particle position %d, %d", x, y);
    particle->phy.pos.x = x;
    particle->phy.pos.y = y;
}
*/

int particle_successor(particle *p) {
    if (p->successor) {
        DEBUG("playing successor animation");
        int direction = p->player.direction;
        //physics_state *phy = &p->phy;
        animationplayer_free(&p->player);
        animationplayer_create(&p->player, 0, p->successor);
        animationplayer_set_direction(&p->player, direction);
        p->player.userdata = p;
        //p->player.phy = phy;
        animationplayer_run(&p->player);
        p->successor = NULL;
        return 1;
    }
    return 0;
}

/*
void particle_wall_hit(physics_state *state, void *userdata, int side, int vstate) {
     particle *p = (particle*)userdata;
     if(!physics_is_bouncy(state) && !particle_successor(p)) {
         p->finished = 1;
     }
     DEBUG("Particle: wall hit @ %d,%d", p->phy.pos.x, p->phy.pos.y);
 }
 
void particle_floor_hit(physics_state *state, void *userdata, int vstate) {
    particle *p = (particle*)userdata;
    if(!physics_is_bouncy(state) && !particle_successor(p)) {
        p->finished = 1;
    }
    DEBUG("Particle: floor hit @ %d,%d", p->phy.pos.x, p->phy.pos.y);
} 

void particle_stopped(physics_state *state, void *userdata) {
    particle *p = (particle*)userdata;
    p->finished = 1;
}
*/

void particle_get_vel(particle *p, int *vx, int *vy) {
    cpVect v = cpBodyGetVel(p->obody);
    *vx = v.x;
    *vy = v.y;
}

void particle_get_pos(particle *p, int *x, int *y) {
    cpVect v = cpBodyGetPos(p->obody);
    *x = v.x;
    *y = v.y;
}

int particle_create(particle *p, unsigned int id, animation *ani, int dir, int x, int y, int vx, int vy, float mass, float gravity, float friction, float elasticity) {
    // Body physics
    cpFloat radius = 5;
    cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
    p->obody = cpSpaceAddBody(p->space, cpBodyNew(mass, moment));
    cpBodySetPos(p->obody, cpv(x, y));
    cpBodySetVel(p->obody, cpv(vx,vy));
    p->oshape = cpSpaceAddShape(p->space, cpCircleShapeNew(p->obody, radius, cpvzero));
    cpShapeSetFriction(p->oshape, friction);
    cpShapeSetElasticity(p->oshape, elasticity);
    cpShapeSetGroup(p->oshape, p->group);
    
    // Animation playback stuff
    animationplayer_create(&p->player, id, ani);
    animationplayer_set_direction(&p->player, dir);
    /*animationplayer_set_repeat(&p->player, 1);*/
    p->player.x = x;
    p->player.y = y;
    p->player.userdata = p;
    p->finished = 0;
    p->id = id;
    p->successor = NULL;
    return 0;
}

void particle_free(particle *p) {
    cpSpaceRemoveShape(p->space, p->oshape);
    cpSpaceRemoveBody(p->space, p->obody);
    cpShapeFree(p->oshape);
    cpBodyFree(p->obody);
    animationplayer_free(&p->player);
}

void particle_tick(particle *p) {
    animationplayer_run(&p->player);
    if(p->player.finished) {
        p->finished = 1;
    }
}

void particle_render(particle *p) {
    cpVect pos = cpBodyGetPos(p->obody);
    p->player.x = pos.x;
    p->player.y = pos.y;
    animationplayer_render(&p->player);
}
