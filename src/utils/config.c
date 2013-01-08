#include "utils/config.h"
#include "utils/log.h"
#include <libconfig.h>

config_t conf;

int conf_init(const char *filename) {
    config_init(&conf);
    if(config_read_file(&conf, filename) == CONFIG_FALSE) {
        ERROR("Error while attempting to read config file!");
        config_destroy(&conf);
        return 1;
    }
    return 0;
}

int conf_int(const char *path) {
    int value;
    config_lookup_int(&conf, path, &value);
    return value;
}

double conf_float(const char *path) {
    double value;
    config_lookup_float(&conf, path, &value);
    return value;
}

int conf_bool(const char *path) {
    int value;
    config_lookup_bool(&conf, path, &value);
    return value;
}

const char* conf_string(const char *path) {
    const char *value;
    config_lookup_string(&conf, path, &value);
    return value;
}

void conf_close() {
    config_destroy(&conf);
}