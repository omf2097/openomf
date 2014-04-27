#ifndef _COMMON_DEFINES_H
#define _COMMON_DEFINES_H

extern const char *difficulty_choices[];
extern const char *round_types[];
extern const char *pilot_names[];
extern const char *har_names[];

enum {
    DIFFICULTY_AI_PUNCHING_BAG = 0,
    DIFFICULTY_AI_ROOKIE,
    DIFFICULTY_AI_VETERAN,
    DIFFICULTY_AI_WORLD_CLASS,
    DIFFICULTY_AI_CHAMPION,
    DIFFICULTY_AI_DEADLY,
    DIFFICULTY_AI_ULTIMATE,
    NUMBER_OF_DIFFICULTY_AI,
};

enum {
    ROUND_TYPE_ONE = 0,
    ROUND_TYPE_2_OF_3,
    ROUND_TYPE_3_OF_5,
    ROUND_TYPE_4_OF_7,
    NUMBER_OF_ROUND_TYPES
};

#endif // _COMMON_DEFINES_H