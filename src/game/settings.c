#include "game/settings.h"
#include "utils/config.h"
#include "utils/log.h"
#include <stddef.h> //offsetof
#include <stdlib.h>
#include <string.h>

settings _settings;

typedef enum field_type_t {
    TYPE_INT, 
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING
} field_type;

typedef struct field_t {
    const char *name;
    field_type type;
    int offset;
} field;

const field f_video[] = {
    {"resindex",    TYPE_INT,  offsetof(settings_video, resindex)},
    {"screen_w",    TYPE_INT,  offsetof(settings_video, screen_w)},
    {"screen_h",    TYPE_INT,  offsetof(settings_video, screen_h)},
    {"vsync",       TYPE_BOOL, offsetof(settings_video, vsync)},
    {"fullscreen",  TYPE_BOOL, offsetof(settings_video, fullscreen)},
    {"scaling",     TYPE_INT,  offsetof(settings_video, scaling)}
};

const field f_sound[] = {
    {"sound_on",           TYPE_BOOL, offsetof(settings_sound, sound_on)},
    {"music_on",           TYPE_BOOL, offsetof(settings_sound, music_on)},
    {"stereo_on",          TYPE_BOOL, offsetof(settings_sound, stereo_on)},
    {"stereo_reversed",    TYPE_BOOL, offsetof(settings_sound, stereo_reversed)},
    {"sound_vol",          TYPE_INT,  offsetof(settings_sound, sound_vol)},
    {"music_vol",          TYPE_INT,  offsetof(settings_sound, music_vol)}
};

const field f_gameplay[] = {
    {"speed",              TYPE_INT,  offsetof(settings_gameplay, speed)},
    {"fight_mode",         TYPE_INT,  offsetof(settings_gameplay, fight_mode)},
    {"power1",             TYPE_INT,  offsetof(settings_gameplay, power1)},
    {"power2",             TYPE_INT,  offsetof(settings_gameplay, power2)},
    {"hazards_on",         TYPE_BOOL, offsetof(settings_gameplay, hazards_on)},
    {"difficulty",         TYPE_INT,  offsetof(settings_gameplay, difficulty)},
    {"rounds",             TYPE_INT,  offsetof(settings_gameplay, rounds)}
};

int *fieldint(void *st, int offset) {
    return (int*)((char*)st + offset);
}
float *fieldfloat(void *st, int offset) {
    return (float*)((char*)st + offset);
}
char* *fieldstr(void *st, int offset) {
    return (char**)((char*)st + offset);
}
int *fieldbool(void *st, int offset) {
    return fieldint(st, offset);
}

void settings_load_fields(void *st, const field *fields, int nfields) {
    for(int i=0;i < nfields;++i) {
        const field *f = &fields[i];
        switch(f->type) {
            case TYPE_INT:
                *fieldint(st, f->offset) = conf_int(f->name);
                break;
                
            case TYPE_FLOAT:
                *fieldfloat(st, f->offset) = conf_float(f->name);
                break;
                
            case TYPE_BOOL:
                *fieldbool(st, f->offset) = conf_bool(f->name);
                break;
                
            case TYPE_STRING:
                {
                    // make a copy of the string
                    char **s = fieldstr(st, f->offset);
                    if(*s) {
                        free(*s);
                    }
                    const char *s2 = conf_string(f->name);
                    *s = malloc(strlen(s2)+1);
                    strcpy(*s, s2);
                }
                break;
        }
    }
}

void settings_save_fields(void *st, const field *fields, int nfields) {
    for(int i=0;i < nfields;++i) {
        const field *f = &fields[i];
        switch(f->type) {
            case TYPE_INT:
                conf_setint(f->name, *fieldint(st, f->offset));
                break;
                
            case TYPE_FLOAT:
                conf_setfloat(f->name, *fieldfloat(st, f->offset));
                break;
                
            case TYPE_BOOL:
                conf_setbool(f->name, *fieldbool(st, f->offset));
                break;
                
            case TYPE_STRING:
                conf_setstring(f->name, *fieldstr(st, f->offset));
                break;
        }
    }
}

void settings_free_strings(void *st, const field *fields, int nfields) {
    for(int i=0;i < nfields;++i) {
        const field *f = &fields[i];
        if(f->type == TYPE_STRING) {
            char **s = fieldstr(st, f->offset);
            if(*s) { 
                free(*s); 
                *s = NULL;
            }
        }
    }
}

int settings_init() {
    memset(&_settings, 0, sizeof(settings));
    return conf_init("openomf.conf");
}

void settings_load() {
    settings_load_fields(&_settings.video, f_video, sizeof(f_video)/sizeof(field));
    settings_load_fields(&_settings.sound, f_sound, sizeof(f_sound)/sizeof(field));
    settings_load_fields(&_settings.gameplay, f_gameplay, sizeof(f_gameplay)/sizeof(field));
}
void settings_save() {
    settings_save_fields(&_settings.video, f_video, sizeof(f_video)/sizeof(field));
    settings_save_fields(&_settings.sound, f_sound, sizeof(f_sound)/sizeof(field));
    settings_save_fields(&_settings.gameplay, f_gameplay, sizeof(f_gameplay)/sizeof(field));
    if(conf_write_config("openomf.conf")) {
        PERROR("Failed to write config file!\n");
    }
}
void settings_free() {
    settings_free_strings(&_settings.video, f_video, sizeof(f_video)/sizeof(field));
    settings_free_strings(&_settings.sound, f_sound, sizeof(f_sound)/sizeof(field));
    settings_free_strings(&_settings.gameplay, f_gameplay, sizeof(f_gameplay)/sizeof(field));
    conf_close();
}

settings *settings_get() {
    return &_settings;
}

