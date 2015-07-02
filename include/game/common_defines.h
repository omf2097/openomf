#ifndef _COMMON_DEFINES_H
#define _COMMON_DEFINES_H

const char* ai_difficulty_get_name(unsigned int id);
const char* har_get_name(unsigned int id);
const char* pilot_get_name(unsigned int id);
const char* round_get_name(unsigned int id);
const char* scene_get_name(unsigned int id);

int har_to_resource(unsigned int id);
int scene_to_resource(unsigned int id);

int rand_arena();

extern const char *ai_difficulty_names[];
extern const char *round_type_names[];
extern const char *pilot_type_names[];
extern const char *har_type_names[];
extern const char *scene_type_names[];

enum {
    SCENE_NONE = 0,
    SCENE_INTRO,
    SCENE_OPENOMF,
    SCENE_MENU,
    SCENE_END,
    SCENE_END1,
    SCENE_END2,
    SCENE_CREDITS,
    SCENE_MECHLAB,
    SCENE_VS,
    SCENE_MELEE,
    SCENE_NEWSROOM,
    SCENE_ARENA0,
    SCENE_ARENA1,
    SCENE_ARENA2,
    SCENE_ARENA3,
    SCENE_ARENA4,
    SCENE_NORTHAM,
    SCENE_KATUSHAI,
    SCENE_WAR,
    SCENE_WORLD,
    SCENE_SCOREBOARD,
    NUMBER_OF_SCENE_TYPES,
};

enum {
    AI_DIFFICULTY_PUNCHING_BAG = 0,
    AI_DIFFICULTY_ROOKIE,
    AI_DIFFICULTY_VETERAN,
    AI_DIFFICULTY_WORLD_CLASS,
    AI_DIFFICULTY_CHAMPION,
    AI_DIFFICULTY_DEADLY,
    AI_DIFFICULTY_ULTIMATE,
    NUMBER_OF_AI_DIFFICULTY_TYPES,
};

// These should match the resource list in order
enum {
    HAR_JAGUAR = 0,
    HAR_SHADOW,
    HAR_THORN,
    HAR_PYROS,
    HAR_ELECTRA,
    HAR_KATANA,
    HAR_SHREDDER,
    HAR_FLAIL,
    HAR_GARGOYLE,
    HAR_CHRONOS,
    HAR_NOVA,
    NUMBER_OF_HAR_TYPES,
};

enum {
    PILOT_CRYSTAL = 0,
    PILOT_STEFFAN,
    PILOT_MILANO,
    PILOT_CHRISTIAN,
    PILOT_SHIRRO,
    PILOT_JEANPAUL,
    PILOT_IBRAHIM,
    PILOT_ANGEL,
    PILOT_COSSETTE,
    PILOT_RAVEN,
    PILOT_KREISSACK,
    NUMBER_OF_PILOT_TYPES,
};

enum {
    ROUND_TYPE_ONE = 0,
    ROUND_TYPE_2_OF_3,
    ROUND_TYPE_3_OF_5,
    ROUND_TYPE_4_OF_7,
    NUMBER_OF_ROUND_TYPES,
};

#endif // _COMMON_DEFINES_H
