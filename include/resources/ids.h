#ifndef _IDS_H
#define _IDS_H

enum RESOURCE_ID {
    SCENE_INTRO = 0,
    SCENE_MENU,
    SCENE_END,
    SCENE_END1,
    SCENE_END2,
    SCENE_CREDITS,
    SCENE_MECHLAB,
    SCENE_VS,
    SCENE_MELEE,
    SCENE_NEWSROOM,
    SCENE_TOURNAMENT,
    SCENE_ARENA0,
    SCENE_ARENA1,
    SCENE_ARENA2,
    SCENE_ARENA3,
    SCENE_ARENA4,
    SCENE_ARENA5,
    SCENE_NORTHAM,
    SCENE_KATUSHAI,
    SCENE_WAR,
    SCENE_WORLD,
    SCENE_NONE,
    HAR_JAGUAR,
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
    HAR_NONE,
};

void get_filename_by_id(int id, char *ptr);
char* get_id_name(int id);

#endif // _IDS_H
