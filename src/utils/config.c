#include "utils/config.h"
#include "utils/log.h"
#include <confuse.h>

cfg_t *cfg=NULL;

cfg_opt_t cfg_opts[] = {
    // video
    CFG_BOOL("vsync",           cfg_false, CFGF_NONE),
    CFG_BOOL("fullscreen",      cfg_false, CFGF_NONE),
    CFG_INT("screen_w",         640,       CFGF_NONE),
    CFG_INT("screen_h",         400,       CFGF_NONE),
    
    // sound
    CFG_BOOL("sound_on",        cfg_true,  CFGF_NONE),
    CFG_BOOL("music_on",        cfg_true,  CFGF_NONE),
    CFG_BOOL("stereo_on",       cfg_true,  CFGF_NONE),
    CFG_BOOL("stereo_reversed", cfg_false, CFGF_NONE),
    
    // gameplay
    CFG_INT("players",          1,         CFGF_NONE),
    CFG_INT("speed",            5,         CFGF_NONE),
    CFG_INT("fight_mode",       0,         CFGF_NONE),
    CFG_INT("power1",           5,         CFGF_NONE),
    CFG_INT("power2",           5,         CFGF_NONE),
    CFG_BOOL("hazards_on",      cfg_true,  CFGF_NONE),
    CFG_INT("difficulty",       1,         CFGF_NONE),
    CFG_INT("rounds",           3,         CFGF_NONE),
    CFG_END()
};

int conf_init(const char *filename) {
    cfg = cfg_init(cfg_opts, 0);
    int ret = cfg_parse(cfg, filename);
    if(ret == CFG_FILE_ERROR) {
        PERROR("Error while attempting to read config file '%s' !", filename);
        cfg_free(cfg);
        return 1;
    } else if(ret == CFG_PARSE_ERROR) {
        PERROR("Error while attempting to parse config file '%s' !", filename);
        cfg_free(cfg);
        return 1;
    }
    DEBUG("Config file '%s' read!", filename);
    return 0;
}

int conf_write_config(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if(fp != NULL) {
        if(cfg == NULL) { 
            cfg_t *tmp = cfg_init(cfg_opts, 0);  
            cfg_print(tmp, fp);
            cfg_free(tmp);
        } else {
            cfg_print(cfg, fp);
        }
        fclose(fp);
        return 0;
    }
    return 1;
}

int conf_int(const char *name) {
    return cfg_getint(cfg, name);
}

double conf_float(const char *name) {
    return cfg_getfloat(cfg, name);
}

int conf_bool(const char *name) {
    return cfg_getbool(cfg, name);
}

const char* conf_string(const char *name) {
    return cfg_getstr(cfg, name);
}

void conf_setint(const char *name, int val) {
    cfg_setint(cfg, name, val);
}

void conf_setfloat(const char *name, double val) {
    cfg_setfloat(cfg, name, val);
}

void conf_setbool(const char *name, int val) {
    cfg_setbool(cfg, name, val);
}

void conf_setstring(const char *name, const char *val) {
    cfg_setstr(cfg, name, val);
}

void conf_close() {
    if(cfg) { 
        cfg_free(cfg); 
        cfg = NULL;
    }
}
