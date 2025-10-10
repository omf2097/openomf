#include "game/common_defines.h"
#include "resources/ids.h"
#include "utils/c_array_util.h"
#include "utils/c_string_util.h"
#include "utils/random.h"
#include "utils/str.h"
#include <stddef.h>
#include <string.h>

const char *ai_difficulty_names[] = {
    "PUNCHING BAG", "ROOKIE", "VETERAN", "WORLD CLASS", "CHAMPION", "DEADLY", "ULTIMATE",
};

const char *round_type_names[] = {
    "ONE ROUND",
    "BEST 2 OF 3",
    "BEST 3 OF 5",
    "BEST 4 OF 7",
};

const char *pilot_type_names[] = {"CRYSTAL", "STEFFAN", "MILANO",   "CHRISTIAN", "SHIRRO",   "JEAN-PAUL",
                                  "IBRAHIM", "ANGEL",   "COSSETTE", "RAVEN",     "KREISSACK"};

const char *har_type_names[] = {"JAGUAR",   "SHADOW", "THORN",    "PYROS",   "ELECTRA", "KATANA",
                                "SHREDDER", "FLAIL",  "GARGOYLE", "CHRONOS", "NOVA"};

const char *scene_type_names[] = {
    "SCENE_NONE",   "SCENE_INTRO",    "SCENE_OPENOMF",      "SCENE_MENU",       "SCENE_END",
    "SCENE_END1",   "SCENE_END2",     "SCENE_CREDITS",      "SCENE_MECHLAB",    "SCENE_VS",
    "SCENE_MELEE",  "SCENE_NEWSROOM", "SCENE_ARENA0",       "SCENE_ARENA1",     "SCENE_ARENA2",
    "SCENE_ARENA3", "SCENE_ARENA4",   "SCENE_TRN_CUTSCENE", "SCENE_SCOREBOARD", "SCENE_LOBBY",
};

int rand_arena(void) {
    return SCENE_ARENA0 + rand_int(5);
}

const char *ai_difficulty_get_name(unsigned int id) {
    if(id >= NUMBER_OF_AI_DIFFICULTY_TYPES) {
        return NULL;
    }
    return ai_difficulty_names[id];
}

const char *har_get_name(unsigned int id) {
    if(id >= NUMBER_OF_HAR_TYPES) {
        return NULL;
    }
    return har_type_names[id];
}

int har_get_id(const char *name) {
    if(!name)
        return -1;

    int num_hars = sizeof(har_type_names) / sizeof(har_type_names[0]);

    for(int id = 0; id < num_hars; id++) {
        if(omf_strncasecmp(name, har_type_names[id], strlen(har_type_names[id])) == 0) {
            return id;
        }
    }

    return -1;
}

const char *pilot_get_name(unsigned int id) {
    if(id >= NUMBER_OF_PILOT_TYPES) {
        return NULL;
    }
    return pilot_type_names[id];
}

const char *round_get_name(unsigned int id) {
    if(id >= NUMBER_OF_ROUND_TYPES) {
        return NULL;
    }
    return round_type_names[id];
}

const char *scene_get_name(unsigned int id) {
    if(id >= NUMBER_OF_SCENE_TYPES) {
        return NULL;
    }
    return scene_type_names[id];
}

int scene_get_id(const char *name) {
    int scene_id = -1;

    str name_upper;
    str_from_c(&name_upper, name);
    str_toupper(&name_upper);
    name = str_c(&name_upper);
    for(unsigned i = 0; i < N_ELEMENTS(scene_type_names); ++i) {
        if(strcmp(scene_type_names[i], name) == 0 || strcmp(scene_type_names[i] + strlen("SCENE_"), name) == 0) {
            scene_id = i;
            break;
        }
    }
    str_free(&name_upper);

    return scene_id;
}

// For these to work, the resources table has to match the ID tables
int har_to_resource(unsigned int id) {
    return AF_JAGUAR + id;
}
