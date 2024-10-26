#ifndef IDS_H
#define IDS_H

typedef enum resource_id
{
    BK_INTRO = 0,
    BK_OPENOMF,
    BK_MENU,
    BK_END,
    BK_END1,
    BK_END2,
    BK_CREDITS,
    BK_MECHLAB,
    BK_VS,
    BK_MELEE,
    BK_NEWSROOM,
    BK_ARENA0,
    BK_ARENA1,
    BK_ARENA2,
    BK_ARENA3,
    BK_ARENA4,
    BK_NORTHAM,
    BK_KATUSHAI,
    BK_WAR,
    BK_WORLD,
    AF_JAGUAR,
    AF_SHADOW,
    AF_THORN,
    AF_PYROS,
    AF_ELECTRA,
    AF_KATANA,
    AF_SHREDDER,
    AF_FLAIL,
    AF_GARGOYLE,
    AF_CHRONOS,
    AF_NOVA,
    PSM_END,
    PSM_MENU,
    PSM_ARENA0,
    PSM_ARENA1,
    PSM_ARENA2,
    PSM_ARENA3,
    PSM_ARENA4,
    DAT_SOUNDS,
    DAT_GRAPHCHR,
    DAT_CHARSMAL,
    DAT_ALTPALS,
    PIC_NORTHAM,
    PIC_KATUSHAI,
    PIC_WAR,
    PIC_WORLD,
    PIC_PLAYERS,
    PCX_NETARENA,
    PCX_NETFONT1,
    PCX_NETFONT2,
    NUMBER_OF_RESOURCES
} resource_id;

const char *get_resource_file(unsigned int id);
const char *get_resource_name(unsigned int id);

int is_arena(unsigned int id);
int is_scene(unsigned int id);
int is_har(unsigned int id);
int is_music(unsigned int id);
int is_pic(unsigned int id);

#endif // IDS_H
