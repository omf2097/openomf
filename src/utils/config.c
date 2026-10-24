#if defined(USE_CONFUSE)

#include "utils/config.h"
#include "utils/log.h"
#include "utils/vector.h"
#include <confuse.h>
#include <string.h>

cfg_t *cfg = NULL;

vector cfg_opts;
int cfg_opts_init = 0;

void conf_ensure_opt_init(void) {
    if(!cfg_opts_init) {
        vector_create_with_size(&cfg_opts, sizeof(cfg_opt_t), 62);
        cfg_opt_t c = CFG_END();
        vector_append(&cfg_opts, &c);
        cfg_opts_init = 1;
    }
}
void conf_append_opt(vector *vec, cfg_opt_t *new_opt) {
    cfg_opt_t *end = vector_get(vec, vector_size(vec) - 1);
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
    cfg = cfg_init((cfg_opt_t *)cfg_opts.data, CFGF_IGNORE_UNKNOWN);
    int ret = cfg_parse(cfg, filename);
    if(ret == CFG_FILE_ERROR) {
        log_error("Error while attempting to read config file '%s' !", filename);
        return 1;
    } else if(ret == CFG_PARSE_ERROR) {
        log_error("Error while attempting to parse config file '%s' !", filename);
        return 1;
    }
    log_info("Configfile read!");
    return 0;
}

int conf_init(const char *filename) {
    int ret = conf_init_internal(filename);
    if(ret == 1) {
        log_debug("Trying to write a default config file.");
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
            cfg_t *tmp = cfg_init((cfg_opt_t *)cfg_opts.data, 0);
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

const char *conf_string(const char *name) {
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

void conf_close(void) {
    if(cfg) {
        cfg_free(cfg);
        cfg = NULL;
    }
    if(cfg_opts_init) {
        vector_free(&cfg_opts);
        cfg_opts_init = 0;
    }
}

#else //  !defined(USE_CONFUSE)

#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/config.h"
#include "utils/iterator.h"
#include "utils/vector.h"

#include <assert.h>

// simple stub config with no on-disk persistance.

typedef enum
{
    STUB_OPT_INT,
    STUB_OPT_BOOL,
    STUB_OPT_FLOAT,
    STUB_OPT_STRING,
} stub_opt_variant;

typedef struct {
    char *name;
    union {
        int value_int;
        int value_bool;
        double value_float;
        char *value_string;
    } value;
    stub_opt_variant variant;
} stub_opt;

static vector cfg_stub_opts;
static int cfg_opts_init = 0;

static void conf_ensure_opt_init(void) {
    if(!cfg_opts_init) {
        vector_create_with_size(&cfg_stub_opts, sizeof(stub_opt), 62);
        cfg_opts_init = 1;
    }
}

static stub_opt *stub_add(char *name, stub_opt_variant variant) {
    conf_ensure_opt_init();
    stub_opt *opt = vector_append_ptr(&cfg_stub_opts);
    opt->name = name;
    opt->variant = variant;
    return opt;
}

static stub_opt *stub_get(char const *name, stub_opt_variant variant) {
    conf_ensure_opt_init();
    iterator it;
    vector_iter_begin(&cfg_stub_opts, &it);
    stub_opt *opt;
    foreach(it, opt) {
        if(variant == opt->variant && strcmp(name, opt->name) == 0)
            return opt;
    }

    assert(!"unknown config entry or mismatched type");
    return NULL;
}

void conf_addint(char *name, int default_val) {
    stub_opt *opt = stub_add(name, STUB_OPT_INT);
    opt->value.value_int = default_val;
}

void conf_addbool(char *name, int default_val) {
    stub_opt *opt = stub_add(name, STUB_OPT_BOOL);
    opt->value.value_bool = default_val;
}

void conf_addfloat(char *name, double default_val) {
    stub_opt *opt = stub_add(name, STUB_OPT_FLOAT);
    opt->value.value_float = default_val;
}

void conf_addstring(char *name, char *default_val) {
    stub_opt *opt = stub_add(name, STUB_OPT_STRING);
    opt->value.value_string = omf_strdup(default_val ? default_val : "");
}

int conf_init(const char *filename) {
    (void)filename; // unused
    return 0;
}

void conf_close(void) {
    if(cfg_opts_init) {
        iterator it;
        vector_iter_begin(&cfg_stub_opts, &it);
        stub_opt *opt;
        foreach(it, opt) {
            if(opt->variant == STUB_OPT_STRING)
                omf_free(opt->value.value_string);
        }

        vector_free(&cfg_stub_opts);
        cfg_opts_init = 0;
    }
}

int conf_write_config(const char *filename) {
    (void)filename; // unused
    return 0;
}

int conf_int(const char *name) {
    stub_opt *opt = stub_get(name, STUB_OPT_INT);
    return opt->value.value_int;
}

double conf_float(const char *name) {
    stub_opt *opt = stub_get(name, STUB_OPT_FLOAT);
    return opt->value.value_float;
}

int conf_bool(const char *name) {
    stub_opt *opt = stub_get(name, STUB_OPT_BOOL);
    return opt->value.value_bool;
}

const char *conf_string(const char *name) {
    stub_opt *opt = stub_get(name, STUB_OPT_STRING);
    return opt->value.value_string;
}

void conf_setint(const char *name, int val) {
    stub_opt *opt = stub_get(name, STUB_OPT_INT);
    opt->value.value_int = val;
}

void conf_setfloat(const char *name, double val) {
    stub_opt *opt = stub_get(name, STUB_OPT_FLOAT);
    opt->value.value_float = val;
}

void conf_setbool(const char *name, int val) {
    stub_opt *opt = stub_get(name, STUB_OPT_BOOL);
    opt->value.value_bool = val;
}

void conf_setstring(const char *name, const char *val) {
    stub_opt *opt = stub_get(name, STUB_OPT_STRING);

    if(val == opt->value.value_string)
        return;

    omf_free(opt->value.value_string);
    opt->value.value_string = omf_strdup(val);
}

#endif // !defined(USE_CONFUSE)
