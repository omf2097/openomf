#include "game/particle.h"

#include "utils/log.h"

void particle_wall_hit(physics_state *state, void *userdata, int side, int vstate) {
    particle *p = (particle*)userdata;
    p->finished = 1;
    DEBUG("Particle: wall hit @ %d,%d", p->phy.pos.x, p->phy.pos.y);
}

void particle_floor_hit(physics_state *state, void *userdata, int vstate) {
    particle *p = (particle*)userdata;
    p->finished = 1;
    DEBUG("Particle: floor hit @ %d,%d", p->phy.pos.x, p->phy.pos.y);
}

int particle_create(particle *p, unsigned int id, animation *ani, int x, int y, int direction, float gravity) {
    physics_init(&p->phy, x, y, 0.0f, 0.0f, 190, 10, 24, 295, gravity, p);
    p->phy.wall_hit = particle_wall_hit;
    p->phy.floor_hit = particle_floor_hit;
    p->phy.vertical_state = PHY_VSTATE_JUMP;
    
    animationplayer_create(&p->player, id, ani);
    animationplayer_set_direction(&p->player, direction);
    animationplayer_set_repeat(&p->player, 1);
    p->finished = 0;
    p->id = id;
    return 0;
}

void particle_free(particle *p) {
    animationplayer_free(&p->player);
}

void particle_tick(particle *p) {
    physics_tick(&p->phy);
    animationplayer_run(&p->player);
}

void particle_render(particle *p) {
    p->player.x = p->phy.pos.x;
    p->player.y = p->phy.pos.y;
    animationplayer_render(&p->player);
}
