#include "game/utils/settings.h"
#include "controller/controller.h"
#include "utils/allocator.h"
#include "utils/config.h"
#include "utils/log.h"
#include <stddef.h> //offsetof
#include <stdlib.h>
#include <string.h>

// clang-format off
#define F_INT(struct_, var, def)                                                                                       \
    { #var, {.i = def }, TYPE_INT, offsetof(struct_, var) }
#define F_BOOL(struct_, var, def)                                                                                      \
    { #var, {.b = def }, TYPE_BOOL, offsetof(struct_, var) }
#define F_FLOAT(struct_, var, def)                                                                                     \
    { #var, {.f = def }, TYPE_FLOAT, offsetof(struct_, var) }
#define F_STRING(struct_, var, def)                                                                                    \
    { #var, {.s = def }, TYPE_STRING, offsetof(struct_, var) }

#define NFIELDS(struct_) sizeof(struct_) / sizeof(field)

#define S_2_F(struct_, field)                                                                                          \
    { struct_, field, NFIELDS(field) }
// clang-format on

static settings _settings;
static const char *settings_path;

typedef enum
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING
} field_type;

typedef union {
    char *s;
    int i;
    int b;
    double f;
} field_default;

typedef struct {
    char *name;
    field_default def;
    field_type type;
    int offset;
} field;

typedef struct {
    void *_struct;
    const field *fields;
    int num_fields;
} struct_to_field;

// clang-format off
const field f_video[] = {
    F_INT(settings_video, screen_w, 640),
    F_INT(settings_video, screen_h, 400),
    F_BOOL(settings_video, vsync, 0),
    F_BOOL(settings_video, fullscreen, 0),
    F_INT(settings_video, scaling, 0),
    F_BOOL(settings_video, instant_console, 0),
    F_BOOL(settings_video, crossfade_on, 1),
};

const field f_sound[] = {
    F_BOOL(settings_sound, music_mono, 0),
    F_INT(settings_sound, sound_vol, 5),
    F_INT(settings_sound, music_vol, 5),
    F_INT(settings_sound, music_frequency, 48000),
    F_INT(settings_sound, music_resampler, 1)
};

const field f_gameplay[] = {
    F_INT(settings_gameplay, speed, 5),
    F_INT(settings_gameplay, fight_mode, 0),
    F_INT(settings_gameplay, power1, 5),
    F_INT(settings_gameplay, power2, 5),
    F_BOOL(settings_gameplay, hazards_on, 1),
    F_INT(settings_gameplay, difficulty, 1),
    F_INT(settings_gameplay, rounds, 1)
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
    F_INT(settings_keyboard, ctrl_type1, CTRL_TYPE_KEYBOARD),
    F_STRING(settings_keyboard, joy_name1, "None"),
    F_INT(settings_keyboard, joy_offset1, -1),
    F_STRING(settings_keyboard, key1_jump_up, "Up"),
    F_STRING(settings_keyboard, key1_jump_right, "PageUp"),
    F_STRING(settings_keyboard, key1_walk_right, "Right"),
    F_STRING(settings_keyboard, key1_duck_forward, "PageDown"),
    F_STRING(settings_keyboard, key1_duck, "Down"),
    F_STRING(settings_keyboard, key1_duck_back, "End"),
    F_STRING(settings_keyboard, key1_walk_back, "Left"),
    F_STRING(settings_keyboard, key1_jump_left, "Home"),
    F_STRING(settings_keyboard, key1_kick, "Right Shift"),
    F_STRING(settings_keyboard, key1_punch, "Return"),
    F_STRING(settings_keyboard, key1_escape, "Escape"),

    // Player two
    F_INT(settings_keyboard, ctrl_type2, CTRL_TYPE_KEYBOARD),
    F_STRING(settings_keyboard, joy_name2, "None"),
    F_INT(settings_keyboard, joy_offset2, -1),
    F_STRING(settings_keyboard, key2_jump_up, "W"),
    F_STRING(settings_keyboard, key2_jump_right, "E"),
    F_STRING(settings_keyboard, key2_walk_right, "D"),
    F_STRING(settings_keyboard, key2_duck_forward, "C"),
    F_STRING(settings_keyboard, key2_duck, "X"),
    F_STRING(settings_keyboard, key2_duck_back, "Z"),
    F_STRING(settings_keyboard, key2_walk_back, "A"),
    F_STRING(settings_keyboard, key2_jump_left, "Q"),
    F_STRING(settings_keyboard, key2_kick, "Left Shift"),
    F_STRING(settings_keyboard, key2_punch, "Left Ctrl"),
    F_STRING(settings_keyboard, key2_escape, "Escape")};

const field f_net[] = {
    F_STRING(settings_network, net_connect_ip, "localhost"),
    F_STRING(settings_network, net_username, ""),
    F_STRING(settings_network, trace_file, NULL),
    F_INT(settings_network, net_connect_port, 2097),
    F_INT(settings_network, net_listen_port_start, 0),
    F_INT(settings_network, net_listen_port_end, 0),
    F_INT(settings_network, net_ext_port_start, 0),
    F_INT(settings_network, net_ext_port_end, 0),
    F_BOOL(settings_network, net_use_pmp, 1),
    F_BOOL(settings_network, net_use_upnp, 1)
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
// clang-format on

int *fieldint(void *st, int offset) {
    return (int *)((char *)st + offset);
}
double *fieldfloat(void *st, int offset) {
    return (double *)((char *)st + offset);
}
char **fieldstr(void *st, int offset) {
    return (char **)((char *)st + offset);
}
int *fieldbool(void *st, int offset) {
    return fieldint(st, offset);
}

void settings_add_fields(const field *fields, int nfields) {
    for(int i = 0; i < nfields; ++i) {
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
    for(int i = 0; i < nfields; ++i) {
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

            case TYPE_STRING: {
                // make a copy of the string
                char **s = fieldstr(st, f->offset);
                omf_free(*s);
                if(conf_string(f->name)) {
                    *s = strdup(conf_string(f->name));
                }
            } break;
        }
    }
}

void settings_save_fields(void *st, const field *fields, int nfields) {
    for(int i = 0; i < nfields; ++i) {
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
                omf_free(*s);
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
    for(unsigned i = 0; i < sizeof(struct_to_fields) / sizeof(struct_to_field); i++) {
        const struct_to_field *s2f = &struct_to_fields[i];
        settings_add_fields(s2f->fields, s2f->num_fields);
    }
    return conf_init(settings_path);
}

void settings_load(void) {
    for(unsigned i = 0; i < sizeof(struct_to_fields) / sizeof(struct_to_field); i++) {
        const struct_to_field *s2f = &struct_to_fields[i];
        settings_load_fields(s2f->_struct, s2f->fields, s2f->num_fields);
    }
}

void settings_save(void) {
    for(unsigned i = 0; i < sizeof(struct_to_fields) / sizeof(struct_to_field); i++) {
        const struct_to_field *s2f = &struct_to_fields[i];
        settings_save_fields(s2f->_struct, s2f->fields, s2f->num_fields);
    }
    if(conf_write_config(settings_path)) {
        PERROR("Failed to write config file!\n");
    }
}

void settings_free(void) {
    for(unsigned i = 0; i < sizeof(struct_to_fields) / sizeof(struct_to_field); i++) {
        const struct_to_field *s2f = &struct_to_fields[i];
        settings_free_strings(s2f->_struct, s2f->fields, s2f->num_fields);
    }
    conf_close();
}

settings *settings_get(void) {
    return &_settings;
}
