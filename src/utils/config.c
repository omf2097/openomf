#include "utils/config.h"
#include "utils/log.h"
#include <confuse.h>

cfg_t *cfg;

cfg_opt_t cfg_opts[] = {
    CFG_BOOL("vsync",      cfg_false, CFGF_NONE),
    CFG_BOOL("fullscreen", cfg_false, CFGF_NONE),
    CFG_INT("screen_w",    640,       CFGF_NONE),
    CFG_INT("screen_h",    400,       CFGF_NONE),
    CFG_END()
};

int conf_init(const char *filename) {
    cfg = cfg_init(cfg_opts, 0);
    int ret = cfg_parse(cfg, filename);
    if(ret == CFG_FILE_ERROR) {
        ERROR("Error while attempting to read config file '%s' !", filename);
        cfg_free(cfg);
        return 1;
    } else if(ret == CFG_PARSE_ERROR) {
        ERROR("Error while attempting to parse config file '%s' !", filename);
        cfg_free(cfg);
        return 1;
    }
    DEBUG("Config file '%s' read!", filename);
    return 0;
}

int conf_write_config(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if(fp != NULL) {
        cfg_t *tmp = cfg_init(cfg_opts, 0);
        cfg_print(tmp, fp);
        cfg_free(tmp);
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

void conf_close() {
    cfg_free(cfg);
}