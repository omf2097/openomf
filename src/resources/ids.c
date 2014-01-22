#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "resources/ids.h"
#include "game/settings.h"
#include "utils/log.h"

static char *filename_mem = NULL;
static char *filename_table[NUMBER_OF_RESOURCES];

struct music_override_t {
    int id;
    const char *name;
};

static const char *get_filename(int id);

void set_default_resource_path() {
#if defined(_WIN32) || defined(WIN32)
    set_resource_path("resources\\");
#else
    set_resource_path("resources/");
#endif
}

void set_resource_path(const char *path) {
    int pathlen = 0;
    int memreq = 0;
    int pos = 0;

    if(filename_mem != NULL) {
        free(filename_mem);
        filename_mem = NULL;
    }
    if(path == NULL) {
        return;
    }

    // Calculate the amount of memory required for the filename table
    pathlen = strlen(path);
    for(int i = 0;i < NUMBER_OF_RESOURCES;i++) {
        const char *fname = get_filename(i);
        if(fname != NULL) {
            memreq += pathlen + strlen(fname) + 1;
        }
    }
    // Allocate the table
    filename_mem = malloc(memreq);

    // Initialize the filename table
    for(int i = 0;i < NUMBER_OF_RESOURCES;i++) {
        const char *fname = get_filename(i);
        int len = 0;
        if(fname == NULL) {
            filename_table[i] = NULL;
        } else {
            len = pathlen + strlen(fname) + 1;
            filename_table[i] = &filename_mem[pos];
            strcpy(filename_table[i], path);
            strcat(filename_table[i], fname);
        }
        pos += len;
    }
}

int validate_resource_path(const char **missingfile) {
    for(int i = 0;i < NUMBER_OF_RESOURCES;i++) {
        *missingfile = get_filename_by_id(i);
        if(*missingfile && access(*missingfile, F_OK ) == -1) {
            return 1;
        }
    }
    return 0;
}

const char *get_filename_by_id(int id) {
    if(filename_mem) {
        return filename_table[id];
    } else {
        return NULL;
    }
}

static const char *get_filename(int id) {
    // Declare music overrides
    settings *s = settings_get();
    struct music_override_t overrides[] = {
        {PSM_ARENA0, s->sound.music_arena0},
        {PSM_ARENA1, s->sound.music_arena1},
        {PSM_ARENA2, s->sound.music_arena2},
        {PSM_ARENA3, s->sound.music_arena3},
        {PSM_ARENA4, s->sound.music_arena4},
        {PSM_MENU,   s->sound.music_menu},
        {PSM_END,    s->sound.music_end}
    };

    for(int i = 0; i < 7; i++) {
        if(id == overrides[i].id && strlen(overrides[i].name) > 0) {
            DEBUG("Overriding %s to %s.", get_id_name(id), overrides[i].name);
            return overrides[i].name;
        }
    }

    switch(id) {
        case SCENE_INTRO:    return "INTRO.BK";
        case SCENE_MENU:     return "MAIN.BK";
        case SCENE_ARENA0:   return "ARENA0.BK";
        case SCENE_ARENA1:   return "ARENA1.BK";
        case SCENE_ARENA2:   return "ARENA2.BK";
        case SCENE_ARENA3:   return "ARENA3.BK";
        case SCENE_ARENA4:   return "ARENA4.BK";
        case SCENE_NEWSROOM: return "NEWSROOM.BK";
        case SCENE_END:      return "END.BK";
        case SCENE_END1:     return "END1.BK";
        case SCENE_END2:     return "END2.BK";
        case SCENE_CREDITS:  return "CREDITS.BK";
        case SCENE_MECHLAB:  return "MECHLAB.BK";
        case SCENE_MELEE:    return "MELEE.BK";
        case SCENE_VS:       return "VS.BK";
        case SCENE_NORTHAM:  return "NORTH_AM.BK";
        case SCENE_KATUSHAI: return "KATUSHAI.BK";
        case SCENE_WAR:      return "WAR.BK";
        case SCENE_WORLD:    return "WORLD.BK";
        case HAR_JAGUAR:     return "FIGHTR0.AF";
        case HAR_SHADOW:     return "FIGHTR1.AF";
        case HAR_THORN:      return "FIGHTR2.AF";
        case HAR_PYROS:      return "FIGHTR3.AF";
        case HAR_ELECTRA:    return "FIGHTR4.AF";
        case HAR_KATANA:     return "FIGHTR5.AF";
        case HAR_SHREDDER:   return "FIGHTR6.AF";
        case HAR_FLAIL:      return "FIGHTR7.AF";
        case HAR_GARGOYLE:   return "FIGHTR8.AF";
        case HAR_CHRONOS:    return "FIGHTR9.AF";
        case HAR_NOVA:       return "FIGHTR10.AF";
        case PSM_MENU:       return "MENU.PSM";
        case PSM_END:        return "END.PSM";
        case PSM_ARENA0:     return "ARENA0.PSM";
        case PSM_ARENA1:     return "ARENA1.PSM";
        case PSM_ARENA2:     return "ARENA2.PSM";
        case PSM_ARENA3:     return "ARENA3.PSM";
        case PSM_ARENA4:     return "ARENA4.PSM";
        case DAT_SOUNDS:     return "SOUNDS.DAT";
        case DAT_ENGLISH:    return "ENGLISH.DAT";
        case DAT_GERMAN:     return "GERMAN.DAT";
        case DAT_GRAPHCHR:   return "GRAPHCHR.DAT";
        case DAT_CHARSMAL:   return "CHARSMAL.DAT";
        case DAT_ALTPALS:    return "ALTPALS.DAT";
        default: break;
    }
    return NULL;
}

const char* get_id_name(int id) {
    switch(id) {
        case SCENE_INTRO:    return "INTRO";
        case SCENE_MENU:     return "MAIN";
        case SCENE_ARENA0:   return "ARENA0";
        case SCENE_ARENA1:   return "ARENA1"; 
        case SCENE_ARENA2:   return "ARENA2"; 
        case SCENE_ARENA3:   return "ARENA3";
        case SCENE_ARENA4:   return "ARENA4";
        case SCENE_NEWSROOM: return "NEWSROOM";
        case SCENE_END:      return "END";
        case SCENE_END1:     return "END1";
        case SCENE_END2:     return "END2";
        case SCENE_CREDITS:  return "CREDITS";
        case SCENE_MECHLAB:  return "MECHLAB";
        case SCENE_MELEE:    return "MELEE";
        case SCENE_VS:       return "VS";
        case SCENE_NORTHAM:  return "NORHAM";
        case SCENE_KATUSHAI: return "KATUSHAI";
        case SCENE_WAR:      return "WAR";
        case SCENE_WORLD:    return "WORLD";
        case HAR_JAGUAR:     return "JAGUAR";
        case HAR_SHADOW:     return "SHADOW";
        case HAR_THORN:      return "THORN";
        case HAR_PYROS:      return "PYROS";
        case HAR_ELECTRA:    return "ELECTRA";
        case HAR_KATANA:     return "KATANA";
        case HAR_SHREDDER:   return "SHREDDER";
        case HAR_FLAIL:      return "FLAIL";
        case HAR_GARGOYLE:   return "GARGOYLE";
        case HAR_CHRONOS:    return "CHRONOS";
        case HAR_NOVA:       return "NOVA";
        case PSM_ARENA0:     return "ARENA0.PSM";
        case PSM_ARENA1:     return "ARENA1.PSM";
        case PSM_ARENA2:     return "ARENA2.PSM";
        case PSM_ARENA3:     return "ARENA3.PSM";
        case PSM_ARENA4:     return "ARENA4.PSM";
        case PSM_MENU:       return "MENU.PSM";
        case PSM_END:        return "END.PSM";
        case DAT_SOUNDS:     return "SOUNDS.DAT";
        case DAT_ENGLISH:    return "ENGLISH.DAT";
        case DAT_GERMAN:     return "GERMAN.DAT";
        case DAT_CHARSMAL:   return "CHARSMAL.DAT";
        case DAT_GRAPHCHR:   return "GRAPHCHR.DAT";
    }
    return "UNKNOWN";
}

int is_arena(int id) {
    return (id >= SCENE_ARENA0 && id <= SCENE_ARENA4);
}

int is_scene(int id) {
    return (id >= SCENE_INTRO && id <= SCENE_WORLD);
}

int is_har(int id) {
    return (id >= HAR_JAGUAR && id <= HAR_NOVA);
}

int is_music(int id) {
    return (id >= PSM_ARENA0 && id <= PSM_END);
}
