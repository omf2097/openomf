#include <stddef.h>
#include "game/common_defines.h"
#include "resources/ids.h"
#include "utils/random.h"

const char *ai_difficulty_names[] = {
    "PUNCHING BAG",
    "ROOKIE",
    "VETERAN",
    "WORLD CLASS",
    "CHAMPION",
    "DEADLY",
    "ULTIMATE",
};

const char *round_type_names[] = {
    "ONE ROUND",
    "BEST 2 OF 3",
    "BEST 3 OF 5",
    "BEST 4 OF 7",
};

const char *pilot_type_names[] = {
    "CRYSTAL",
    "STEFFAN",
    "MILANO",
    "CHRISTIAN",
    "SHIRRO",
    "JEAN-PAUL",
    "IBRAHIM",
    "ANGEL",
    "COSSETTE",
    "RAVEN",
    "KREISSACK"
};

const char *har_type_names[] = {
    "JAGUAR",
    "SHADOW",
    "THORN",
    "PYROS",
    "ELECTRA",
    "KATANA",
    "SHREDDER",
    "FLAIL",
    "GARGOYLE",
    "CHRONOS",
    "NOVA"
};

const char *scene_type_names[] = {
    "SCENE_NONE",
    "SCENE_INTRO",
    "SCENE_OPENOMF",
    "SCENE_MENU",
    "SCENE_END",
    "SCENE_END1",
    "SCENE_END2",
    "SCENE_CREDITS",
    "SCENE_MECHLAB",
    "SCENE_VS",
    "SCENE_MELEE",
    "SCENE_NEWSROOM",
    "SCENE_ARENA0",
    "SCENE_ARENA1",
    "SCENE_ARENA2",
    "SCENE_ARENA3",
    "SCENE_ARENA4",
    "SCENE_NORTHAM",
    "SCENE_KATUSHAI",
    "SCENE_WAR",
    "SCENE_WORLD",
    "SCENE_SCOREBOARD",
};

int rand_arena() {
   return SCENE_ARENA0 + rand_int(5);
}

const char* ai_difficulty_get_name(unsigned int id) {
    if(id >= NUMBER_OF_AI_DIFFICULTY_TYPES) {
        return NULL;
    }
    return ai_difficulty_names[id];
}

const char* har_get_name(unsigned int id) {
    if(id >= NUMBER_OF_HAR_TYPES) {
        return NULL;
    }
    return har_type_names[id];
}

const char* pilot_get_name(unsigned int id) {
    if(id >= NUMBER_OF_PILOT_TYPES) {
        return NULL;
    }
    return pilot_type_names[id];
}

const char* round_get_name(unsigned int id) {
    if(id >= NUMBER_OF_ROUND_TYPES) {
        return NULL;
    }
    return round_type_names[id];
}

const char* scene_get_name(unsigned int id) {
    if(id >= NUMBER_OF_SCENE_TYPES) {
        return NULL;
    }
    return scene_type_names[id];
}

// For these to work, the resources table has to match the ID tables
int har_to_resource(unsigned int id) {
    return AF_JAGUAR + id;
}

int scene_to_resource(unsigned int id) {
    switch(id) {
        case SCENE_SCOREBOARD:
            return BK_MENU;
        default:
            return BK_INTRO + (id - 1);
    }
}
