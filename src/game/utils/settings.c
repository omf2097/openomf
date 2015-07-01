#include "game/utils/settings.h"
#include "controller/controller.h"
#include "utils/config.h"
#include "utils/log.h"
#include <stddef.h> //offsetof
#include <stdlib.h>
#include <string.h>

#define F_INT(struct_, var, def) {#var, TYPE_INT, {.i=def}, offsetof(struct_, var)}
#define F_BOOL(struct_, var, def) {#var, TYPE_BOOL, {.b=def}, offsetof(struct_, var)}
#define F_FLOAT(struct_, var, def) {#var, TYPE_FLOAT, {.f=def}, offsetof(struct_, var)}
#define F_STRING(struct_, var, def) {#var, TYPE_STRING, {.s=def}, offsetof(struct_, var)}

#define NFIELDS(struct_) sizeof(struct_)/sizeof(field)

#define S_2_F(struct_, field) {struct_, field, NFIELDS(field)}

static settings _settings;
static const char *settings_path;

typedef enum field_type_t {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING
} field_type;

typedef union field_default_u {
    char *s;
    int i;
    int b;
    double f;
} field_default;

typedef struct field_t {
    char *name;
    field_type type;
    field_default def;
    int offset;
} field;

typedef struct struct_to_field_t {
    void *_struct;
    const field *fields;
    int num_fields;
} struct_to_field;

const field f_video[] = {
    F_INT(settings_video,  screen_w,       640),
    F_INT(settings_video,  screen_h,       400),
    F_BOOL(settings_video, vsync,            0),
    F_BOOL(settings_video, fullscreen,       0),
    F_INT(settings_video,  scaling,          0),
    F_BOOL(settings_video, instant_console,  0),
    F_BOOL(settings_video, crossfade_on,     1),
    F_STRING(settings_video, scaler, "Nearest"),
    F_INT(settings_video,  scale_factor,     1),
};

const field f_sound[] = {
    F_STRING(settings_sound, sink,            "openal"),
    F_BOOL(settings_sound, music_mono,        0),
    F_INT(settings_sound,  sound_vol,         5),
    F_INT(settings_sound,  music_vol,         5),
    F_INT(settings_sound,  music_frequency,   44100),
#if USE_DUMB
    F_INT(settings_sound, music_library,      0),
    F_INT(settings_sound, music_resampler,    2),
#elif USE_XMP
    F_INT(settings_sound, music_library,      2),
    F_INT(settings_sound, music_resampler,    1),
#elif USE_MODPLUG
    F_INT(settings_sound, music_library,      1),
    F_INT(settings_sound, music_resampler,    1),
#endif
    F_STRING(settings_sound, music_arena0,    ""),
    F_STRING(settings_sound, music_arena1,    ""),
    F_STRING(settings_sound, music_arena2,    ""),
    F_STRING(settings_sound, music_arena3,    ""),
    F_STRING(settings_sound, music_arena4,    ""),
    F_STRING(settings_sound, music_end,       ""),
    F_STRING(settings_sound, music_menu,      "")
};

const field f_gameplay[] = {
    F_INT(settings_gameplay,  speed,       5),
    F_INT(settings_gameplay,  fight_mode,  0),
    F_INT(settings_gameplay,  power1,      5),
    F_INT(settings_gameplay,  power2,      5),
    F_BOOL(settings_gameplay, hazards_on,  1),
    F_INT(settings_gameplay,  difficulty,  1),
    F_INT(settings_gameplay,  rounds,      1)
};

const field f_tournament[] = {
    F_STRING(settings_tournament, last_name, ""),
};

const field f_advanced[] = {
    F_INT(settings_advanced, rehit_mode, 0),
    F_INT(settings_advanced, defensive_throws, 0),
    F_INT(settings_advanced, throw_range, 100),
    F_INT(settings_advanced, jump_height, 100),
    F_INT(settings_advanced, hit_pause, 0),
    F_INT(settings_advanced, vitality, 100),
    F_INT(settings_advanced, knock_down, KNOCK_DOWN_BOTH),
    F_INT(settings_advanced, block_damage, 0),
};

const field f_keyboard[] = {
    // Player one
    F_INT(settings_keyboard, ctrl_type1,  CTRL_TYPE_KEYBOARD),
    F_STRING(settings_keyboard, joy_name1,  "None"),
    F_INT(settings_keyboard, joy_offset1,  -1),
    F_STRING(settings_keyboard, key1_jump_up,    "Up"),
    F_STRING(settings_keyboard, key1_jump_right,  "PageUp"),
    F_STRING(settings_keyboard, key1_walk_right,  "Right"),
    F_STRING(settings_keyboard, key1_duck_forward, "PageDown"),
    F_STRING(settings_keyboard, key1_duck,    "Down"),
    F_STRING(settings_keyboard, key1_duck_back,  "End"),
    F_STRING(settings_keyboard, key1_walk_back,  "Left"),
    F_STRING(settings_keyboard, key1_jump_left, "Home"),
    F_STRING(settings_keyboard, key1_kick,  "Right Shift"),
    F_STRING(settings_keyboard, key1_punch, "Return"),
    F_STRING(settings_keyboard, key1_escape, "Escape"),

    // Player two
    F_INT(settings_keyboard, ctrl_type2,  CTRL_TYPE_KEYBOARD),
    F_STRING(settings_keyboard, joy_name2,  "None"),
    F_INT(settings_keyboard, joy_offset2,  -1),
    F_STRING(settings_keyboard, key2_jump_up,    "W"),
    F_STRING(settings_keyboard, key2_jump_right,  "E"),
    F_STRING(settings_keyboard, key2_walk_right,  "D"),
    F_STRING(settings_keyboard, key2_duck_forward, "C"),
    F_STRING(settings_keyboard, key2_duck,    "X"),
    F_STRING(settings_keyboard, key2_duck_back,  "Z"),
    F_STRING(settings_keyboard, key2_walk_back,  "A"),
    F_STRING(settings_keyboard, key2_jump_left, "Q"),
    F_STRING(settings_keyboard, key2_kick,  "Left Shift"),
    F_STRING(settings_keyboard, key2_punch, "Left Ctrl"),
    F_STRING(settings_keyboard, key2_escape, "Escape")
};

const field f_net[] = {
    F_STRING(settings_network, net_connect_ip,   "localhost"),
    F_INT(settings_network,    net_connect_port, 2097),
    F_INT(settings_network,    net_listen_port, 2097)
};

// Map struct to field
const struct_to_field struct_to_fields[] = {
    S_2_F(&_settings.video, f_video),
    S_2_F(&_settings.sound, f_sound),
    S_2_F(&_settings.gameplay, f_gameplay),
    S_2_F(&_settings.tournament, f_tournament),
    S_2_F(&_settings.advanced, f_advanced),
    S_2_F(&_settings.keys, f_keyboard),
    S_2_F(&_settings.net, f_net)
};

int *fieldint(void *st, int offset) {
    return (int*)((char*)st + offset);
}
double *fieldfloat(void *st, int offset) {
    return (double*)((char*)st + offset);
}
char* *fieldstr(void *st, int offset) {
    return (char**)((char*)st + offset);
}
int *fieldbool(void *st, int offset) {
    return fieldint(st, offset);
}

void settings_add_fields(const field *fields, int nfields) {
    for(int i=0;i < nfields;++i) {
        const field *f = &fields[i];
        switch(f->type) {
            case TYPE_INT:
                conf_addint(f->name, f->def.i);
                break;

            case TYPE_FLOAT:
                conf_addfloat(f->name, f->def.f);
                break;

            case TYPE_BOOL:
                conf_addbool(f->name, f->def.b);
                break;

            case TYPE_STRING:
                conf_addstring(f->name, f->def.s);
                break;
        }
    }
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
    for(int i = 0; i < nfields; ++i) {
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

int settings_write_defaults(const char *path) {
    int r = 0;
    settings_init(path);
    if(conf_write_config(path)) {
        r = 1;
    }
    settings_free();
    return r;
}

int settings_init(const char *path) {
    settings_path = path;
    memset(&_settings, 0, sizeof(settings));
    for(int i = 0;i < sizeof(struct_to_fields)/sizeof(struct_to_field);i++) {
        const struct_to_field *s2f = &struct_to_fields[i];
        settings_add_fields(s2f->fields, s2f->num_fields);
    }
    return conf_init(settings_path);
}

void settings_load() {
    for(int i = 0;i < sizeof(struct_to_fields)/sizeof(struct_to_field);i++) {
        const struct_to_field *s2f = &struct_to_fields[i];
        settings_load_fields(s2f->_struct, s2f->fields, s2f->num_fields);
    }
}

void settings_save() {
    for(int i = 0;i < sizeof(struct_to_fields)/sizeof(struct_to_field);i++) {
        const struct_to_field *s2f = &struct_to_fields[i];
        settings_save_fields(s2f->_struct, s2f->fields, s2f->num_fields);
    }
    if(conf_write_config(settings_path)) {
        PERROR("Failed to write config file!\n");
    }
}

void settings_free() {
    for(int i = 0;i < sizeof(struct_to_fields)/sizeof(struct_to_field);i++) {
        const struct_to_field *s2f = &struct_to_fields[i];
        settings_free_strings(s2f->_struct, s2f->fields, s2f->num_fields);
    }
    conf_close();
}

settings *settings_get() {
    return &_settings;
}
