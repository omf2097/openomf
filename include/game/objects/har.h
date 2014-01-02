#ifndef _HAR_H
#define _HAR_H

#include "resources/af.h"
#include "resources/animation.h"
#include "game/protos/object.h"
#include "utils/list.h"

// For debug texture stuff
#ifdef DEBUGMODE
#include "video/texture.h"
#include "video/image.h"
#endif

#define LAYER_HAR 0x02
#define LAYER_HAR1 0x04
#define LAYER_HAR2 0x08
#define LAYER_SCRAP 0x10
#define LAYER_PROJECTILE 0x20

#define GROUP_PROJECTILE 2

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
    STATE_STANDING,
    STATE_WALKING,
    STATE_CROUCHING,
    STATE_JUMPING,
    STATE_RECOIL,
    STATE_FALLEN,
    STATE_STANDING_UP,
    STATE_STUNNED,
    STATE_VICTORY,
    STATE_DEFEAT,
    STATE_SCRAP,
    STATE_DESTRUCTION,
    STATE_DONE // destruction or scrap has completed
};

enum {
    DAMAGETYPE_LOW, // Damage to low area of har
    DAMAGETYPE_HIGH // Damage to high area of har
};

typedef void (*har_hook_cb)(int action, void *data);

typedef struct har_t {
    uint8_t id;
    uint8_t player_id;
    uint8_t pilot_id;
    uint8_t state;
    uint8_t blocking;
    uint8_t executing_move;
    uint8_t flinching;
    uint8_t close;
    af *af_data;
    uint8_t damage_done; // Damage was done this animation
    uint8_t damage_received; // Damage was received this animation

    int16_t health_max, health;
    int16_t endurance_max, endurance;
    char inputs[11];
    uint8_t hard_close;

    har_hook_cb hook_cb;
    void *hook_cb_data;


#ifdef DEBUGMODE
    texture debug_tex;
    image debug_img;
    uint8_t debug_enabled;
#endif
} har;

void har_install_hook(har *h, har_hook_cb hook, void *data);
void har_bootstrap(object *obj);
int har_create(object *obj, af *af_data, int dir, int har_id, int pilot_id, int player_id);
void har_set_ani(object *obj, int animation_id, int repeat);
int har_is_active(object *obj);

#endif // _HAR_H
