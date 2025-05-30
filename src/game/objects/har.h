#ifndef HAR_H
#define HAR_H

#include "game/objects/arena_constraints.h"
#include "game/protos/object.h"
#include "resources/af.h"
#include "resources/animation.h"
#include "resources/bk.h"
#include "utils/list.h"

// For debug texture stuff
#ifdef DEBUGMODE
#include "video/image.h"
#include "video/surface.h"
#endif

#define LAYER_HAR 0x02
#define LAYER_HAR1 0x04
#define LAYER_HAR2 0x08
#define LAYER_SCRAP 0x10
#define LAYER_PROJECTILE 0x20
#define LAYER_HAZARD 0x40

#define STUN_RECOVERY_CONSTANT 256 * 18 / 250
#define STUN_RECOVERY_BLOCKING_CONSTANT 256 * 27 / 250
#define HEIGHT_STANDING 55
#define HEIGHT_CROUCHING 30

enum
{
    CAT_MISC = 0,
    CAT_CLOSE = 2,
    CAT_LOW = 4,
    CAT_MEDIUM = 5,
    CAT_HIGH,
    CAT_JUMPING,
    CAT_PROJECTILE,
    CAT_BASIC,
    CAT_VICTORY, // or defeat
    CAT_FIRE_ICE,
    CAT_SCRAP,
    CAT_DESTRUCTION
};

enum
{
    STATE_NONE = 0,
    STATE_STANDING = 1,
    STATE_WALKTO,
    STATE_WALKFROM,
    STATE_CROUCHING,
    STATE_CROUCHBLOCK,
    STATE_JUMPING,
    STATE_RECOIL,
    STATE_STANDING_UP,
    STATE_STUNNED,
    STATE_BLOCKSTUN,
    STATE_VICTORY,
    STATE_DEFEAT,
    STATE_SCRAP,
    STATE_DESTRUCTION,
    STATE_WALLDAMAGE, // Took damage from wall (electrocution)
    STATE_DONE        // destruction or scrap has completed
};

enum
{
    HAR_EVENT_JUMP,
    HAR_EVENT_AIR_TURN,
    HAR_EVENT_WALK,
    HAR_EVENT_AIR_ATTACK_DONE,        // Touched the floor after executing a move in the air
    HAR_EVENT_ATTACK,                 // Executed a move, may not hit
    HAR_EVENT_ENEMY_BLOCK,            // Opponent blocked your move
    HAR_EVENT_ENEMY_BLOCK_PROJECTILE, // Opponent blocked your projectile
    HAR_EVENT_BLOCK,                  // You blocked your opponents move
    HAR_EVENT_BLOCK_PROJECTILE,       // You blocked your opponents projectile
    HAR_EVENT_LAND_HIT,               // Landed a hit on the opponent
    HAR_EVENT_LAND_HIT_PROJECTILE,    // Landed a projectile hit on the opponent
    HAR_EVENT_TAKE_HIT,               // Hit by HAR
    HAR_EVENT_TAKE_HIT_PROJECTILE,    // Hit by projectile
    HAR_EVENT_HAZARD_HIT,             // Hit by hazard
    HAR_EVENT_ENEMY_HAZARD_HIT,
    HAR_EVENT_STUN,
    HAR_EVENT_ENEMY_STUN,
    HAR_EVENT_RECOVER,  // regained control after recoil/stun
    HAR_EVENT_HIT_WALL, // Touched a wall
    HAR_EVENT_LAND,     // Touched the floor
    HAR_EVENT_DEFEAT,
    HAR_EVENT_SCRAP,
    HAR_EVENT_DESTRUCTION,
    HAR_EVENT_DONE // match done, no more scrap/destruction
};

typedef struct har_event_t {
    uint8_t type;
    uint8_t player_id;
    union {
        af_move *move; // for attack/hit
        bk_info *info; // for hazard hit
        int wall;      // for hit wall
        int direction; // jump direction
    };
} har_event;

enum
{
    DAMAGETYPE_LOW, // Damage to low area of har
    DAMAGETYPE_HIGH // Damage to high area of har
};

typedef void (*har_action_hook_cb)(int action, void *data);
typedef void (*har_hook_cb)(har_event event, void *data);

typedef struct har_hook_t {
    har_hook_cb cb;
    void *data;
} har_hook;

typedef struct action_buffer_t {
    char actions[10];
    uint8_t count;
    uint32_t age;
} action_buffer;

typedef struct game_player_t game_player;

typedef struct har_t {
    uint8_t id;        // Har ID
    uint8_t player_id; // Player number, 0 or 1
    uint8_t pilot_id;  // Pilot ID
    uint8_t state;
    uint8_t executing_move;
    uint8_t close; // This helps the AI
    const af *af_data;
    uint8_t damage_done;     // Damage was done this animation
    uint8_t damage_received; // Damage was received this animation
    uint8_t air_attacked;
    uint8_t is_wallhugging;  // HAR is standing right next to a wall
    uint8_t is_grabbed;      // Is being moved by another object. Set by ex, ey tags
    float last_damage_value; // Last damage value taken
    float last_stun_value;   // Last stun value taken

    float jump_speed;      // Agility generated speed modifier for jumping
    float superjump_speed; // Agility generated speed modifier for jumping
    float fall_speed;      // Agility generated speed modifier for falling
    float fwd_speed;       // Agility generated speed modifier for falling
    float back_speed;      // Agility generated speed modifier for falling

    int in_stasis_ticks; // Handle stasis activator
    int throw_duration;
    int block_duration;
    int height; // Distance required to jump over this HAR

    uint8_t stride;
    int stun_factor;
    int16_t health_max, health;
    int endurance_max, endurance;
    char inputs[11];
    uint32_t input_change_tick; // last tick the input direction changed

    uint8_t stun_timer;
    uint8_t delay; // used for 'stretching' frames in netplay

    // ptr, pe, etc. stuff
    uint8_t p_pal_ref;    // "pd"
    uint8_t p_har_switch; // "pe"
    int16_t p_fade_out_ticks;
    int16_t p_fade_out_ticks_left;
    int16_t p_fade_in_ticks;
    int16_t p_fade_in_ticks_left;
    int16_t p_sustain_ticks_left;
    // int16_t p_base_intensity; // "pb"
    // int16_t p_max_intensity;  // "pp"
    uint8_t p_color_fn;

    int walk_destination;

    int walk_done_anim;
    int walk_done_tick;

    uint8_t custom_defeat_animation;

    char rehits[20]; // list of the move IDs that cannot rehit again
    bool rehit_combo;

    list har_hooks;

    hashmap disabled_animations;
    hashmap *trail_cache;

#ifdef DEBUGMODE
    surface hit_pixel;
    surface har_origin;
#endif
} har;

void har_install_action_hook(har *h, har_action_hook_cb hook, void *data);
void har_install_hook(har *h, har_hook_cb hook, void *data);
void har_bootstrap(object *obj);
int har_create(object *obj, af *af_data, int dir, int har_id, int pilot_id, int player_id);
void har_face_enemy(object *obj, object *obj_enemy);
void har_set_ani(object *obj, int animation_id, int repeat);
void har_walk_to(object *obj, int destination);
int har_is_active(object *obj);
int har_is_crouching(har *h);
int har_is_walking(har *h);
int har_is_blocking(object *obj, af_move *move);
void har_copy_actions(object *new, object *old);
void har_reset(object *obj);

void har_set_delay(object *obj, int delay);

uint8_t har_player_id(object *obj);

int16_t har_health_percent(har *h);

void cb_har_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g, void *userdata);
void cb_har_disable_animation(object *parent, uint8_t animation_id, uint16_t ticks, void *userdata);

#endif // HAR_H
