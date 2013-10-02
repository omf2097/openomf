#ifndef _HAR_H
#define _HAR_H

#include "resources/af.h"
#include "game/protos/object.h"
#include "utils/list.h"

#define LAYER_HAR 0x01
#define LAYER_SCRAP 0x02

// All HARs have these predefined animations
enum {
    ANIM_JUMPING=1,
    ANIM_STANDUP,
    ANIM_STUNNED,
    ANIM_CROUCHING,
    ANIM_STANDING_BLOCK,
    ANIM_CROUCHING_BLOCK,
    ANIM_BURNING_OIL,
    ANIM_BLOCKING_SCRAPE,
    ANIM_DAMAGE,
    ANIM_WALKING,
    ANIM_IDLE,
    ANIM_SCRAP_METAL,
    ANIM_BOLT,
    ANIM_SCREW,
    ANIM_VICTORY=48,
    ANIM_DEFEAT,
    ANIM_BLAST1=55,
    ANIM_BLAST2,
    ANIM_BLAST3
};

typedef struct har_t {
    int id;
    af af_data;
    unsigned int state;

    int health_max, health;
    int endurance_max, endurance;
    char inputs[10];
} har;

int har_create(object *obj, palette *pal, int dir, int har_id);
void har_tick(object *obj); 
void har_act(object *obj, int act_type);
void har_free(object *obj);
void har_finished(object *obj);

#endif // _HAR_H
