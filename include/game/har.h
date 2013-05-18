#ifndef _HAR_H
#define _HAR_H

#include <shadowdive/shadowdive.h>
#include "utils/array.h"
#include "game/animation.h"
#include "game/animationplayer.h"
//#include "game/scene.h"
#include "game/physics/physics.h"

enum {
    ACT_KICK,
    ACT_PUNCH,
    ACT_UP,
    ACT_UPLEFT,
    ACT_UPRIGHT,
    ACT_DOWN,
    ACT_DOWNLEFT,
    ACT_DOWNRIGHT,
    ACT_LEFT,
    ACT_RIGHT,
    ACT_STOP,
    ACT_WALKLEFT,
    ACT_WALKRIGHT,
    ACT_CROUCH,
    ACT_JUMP
};

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

// these are mostly guesses, but seem to fit
enum {
    CAT_MISC = 0,
    CAT_CLOSE = 2,
    CAT_CROUCH = 4,
    CAT_STANDING = 5,
    CAT_WALKING, // may also include standing
    CAT_JUMPING,
    CAT_PROJECTILE,
    CAT_BASIC,
    CAT_VICTORY = 10, // or defeat
    CAT_SCRAP = 12,
    CAT_DESTRUCTION
};

enum {
    HAR_JAGUAR=0,
    HAR_SHADOW,
    HAR_THORN,
    HAR_PYROS,
    HAR_ELECTRA,
    HAR_KATANA,
    HAR_SHREDDER,
    HAR_FLAIL,
    HAR_GARGOYLE,
    HAR_CHRONOS,
    HAR_NOVA
};

enum {
    STATE_STANDING,
    STATE_WALKING,
    STATE_CROUCHING,
    STATE_JUMPING,
    STATE_RECOIL,
    STATE_VICTORY,
    STATE_SCRAP,
    STATE_DESTRUCTION
};

typedef struct har_t {
    physics_state phy;
    unsigned int state;
    int direction; // 1 or -1
    sd_af_file *af;
    array animations;
    animationplayer player;
    char inputs[11]; // I don't think any move in the game needs 10 inputs to trigger...
    
    int health, health_max;
    int endurance, endurance_max;
    
    int tick; // TEMPORARY TO SLOW DOWN ANIMATION

    unsigned int cd_debug_enabled;
    image cd_debug;
    texture cd_debug_tex;

    int close; // are we close to the other player?

    list child_players; // projectiles and the like
} har;

void har_free(har *har);
int har_load(har *har, sd_palette *pal, int id, int x, int y, int direction); // Returns 0 on success
void har_tick(har *har); // Called by scene.c tick function at every game tick
void har_render(har *har); // Called by scene.h render function at every frame render
void har_act(har *har, int act_type); // Handle event passed from inputhandler
void har_set_direction(har *har, int direction);
void har_collision_har(har *har_a, har *har_b);
void har_take_damage(har *har, int amount, const char *string);

#endif // _HAR_H
