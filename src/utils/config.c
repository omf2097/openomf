#include "utils/config.h"
#include "utils/log.h"
#include "utils/vector.h"
#include <string.h>
#include <confuse.h>

cfg_t *cfg = NULL;

vector cfg_opts;
int cfg_opts_init = 0;

void conf_ensure_opt_init() {
    if(!cfg_opts_init) {
        vector_create(&cfg_opts, sizeof(cfg_opt_t));
        cfg_opt_t c = CFG_END();
        vector_append(&cfg_opts, &c);
        cfg_opts_init = 1;
    }
}
void conf_append_opt(vector *vec, cfg_opt_t *new_opt) {
    cfg_opt_t *end = vector_get(vec, vector_size(vec)-1);
    memcpy(end, new_opt, sizeof(cfg_opt_t));
    cfg_opt_t c = CFG_END();
    vector_append(vec, &c);
}

void conf_addint(char *name, int default_val) {
    conf_ensure_opt_init();
    cfg_opt_t new_opt = CFG_INT(name, default_val, CFGF_NONE);
    conf_append_opt(&cfg_opts, &new_opt);
}
void conf_addbool(char *name, int default_val) {
    conf_ensure_opt_init();
    cfg_opt_t new_opt = CFG_BOOL(name, default_val, CFGF_NONE);
    conf_append_opt(&cfg_opts, &new_opt);
}
void conf_addfloat(char *name, double default_val) {
    conf_ensure_opt_init();
    cfg_opt_t new_opt = CFG_FLOAT(name, default_val, CFGF_NONE);
    conf_append_opt(&cfg_opts, &new_opt);
}
void conf_addstring(char *name, char *default_val) {
    conf_ensure_opt_init();
    cfg_opt_t new_opt = CFG_STR(name, default_val, CFGF_NONE);
    conf_append_opt(&cfg_opts, &new_opt);
}

int conf_init_internal(const char *filename) {
    conf_ensure_opt_init();
    cfg = cfg_init((cfg_opt_t*)cfg_opts.data, 0);
    int ret = cfg_parse(cfg, filename);
    if(ret == CFG_FILE_ERROR) {
        PERROR("Error while attempting to read config file '%s' !", filename);
        return 1;
    } else if(ret == CFG_PARSE_ERROR) {
        PERROR("Error while attempting to parse config file '%s' !", filename);
        return 1;
    }
    DEBUG("Config file '%s' read!", filename);
    return 0;
}

int conf_init(const char *filename) {
    int ret = conf_init_internal(filename);
    if(ret == 1) {
        DEBUG("Trying to write a default config file.");
        if(!conf_write_config(filename)) {
            return conf_init_internal(filename);
        } else {
            goto error_0;
        }
    }
    return 0;

error_0:
    cfg_free(cfg);
    return 1;
}

int conf_write_config(const char *filename) {
    conf_ensure_opt_init();
    FILE *fp = fopen(filename, "w");
    if(fp != NULL) {
        if(cfg == NULL) { 
            cfg_t *tmp = cfg_init((cfg_opt_t*)cfg_opts.data, 0);  
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
    cfg_setbool(cfg, name, (cfg_bool_t)val);
}

void conf_setstring(const char *name, const char *val) {
    cfg_setstr(cfg, name, val);
}

void conf_close() {
    if(cfg) { 
        cfg_free(cfg); 
        cfg = NULL;
    }
    if(cfg_opts_init) {
        vector_free(&cfg_opts);
        cfg_opts_init = 0;
    }
}
