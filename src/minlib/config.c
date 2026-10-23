/* Built-in settings for MIN_LIB_BUILD. Values registered with conf_add* are the config. */
#ifndef MIN_LIB_BUILD
#error "config.c stand-in is part of MIN_LIB_BUILD"
#endif

#include "utils/config.h"
#include "utils/log.h"
#include "utils/vector.h"

#include <stdlib.h>
#include <string.h>

typedef enum conf_kind
{
    CONF_INT,
    CONF_BOOL,
    CONF_FLOAT,
    CONF_STRING
} conf_kind;

typedef struct conf_item {
    const char *name;
    conf_kind kind;
    long i;
    double f;
    char *s;
    int own_s;
} conf_item;

static vector items;
static int ready;

static char *dup_string(const char *text) {
    size_t n;
    char *copy;
    n = strlen(text);
    copy = (char *)malloc(n + 1);
    if(copy == NULL) {
        return NULL;
    }
    memcpy(copy, text, n + 1);
    return copy;
}

static void ensure_items(void) {
    if(!ready) {
        vector_create(&items, sizeof(conf_item));
        ready = 1;
    }
}

static conf_item *find_item(const char *name) {
    unsigned int i;
    if(!ready || name == NULL) {
        return NULL;
    }
    for(i = 0; i < vector_size(&items); i++) {
        conf_item *item = vector_get(&items, i);
        if(item->name != NULL && strcmp(item->name, name) == 0) {
            return item;
        }
    }
    return NULL;
}

static void add_item(const char *name, conf_kind kind, long i, double f, char *s) {
    conf_item item;
    ensure_items();
    memset(&item, 0, sizeof(item));
    item.name = name;
    item.kind = kind;
    item.i = i;
    item.f = f;
    item.s = s;
    vector_append(&items, &item);
}

void conf_addint(char *name, int default_val) {
    add_item(name, CONF_INT, default_val, 0.0, NULL);
}

void conf_addbool(char *name, int default_val) {
    add_item(name, CONF_BOOL, default_val ? 1 : 0, 0.0, NULL);
}

void conf_addfloat(char *name, double default_val) {
    add_item(name, CONF_FLOAT, 0, default_val, NULL);
}

void conf_addstring(char *name, char *default_val) {
    add_item(name, CONF_STRING, 0, 0.0, default_val);
}

int conf_init(const char *filename) {
    (void)filename;
    log_info("Using built-in default settings.");
    return 0;
}

int conf_write_config(const char *filename) {
    (void)filename;
    log_info("Built-in settings are not written to disk.");
    return 0;
}

int conf_int(const char *name) {
    conf_item *item = find_item(name);
    if(item == NULL) {
        return 0;
    }
    return (int)item->i;
}

double conf_float(const char *name) {
    conf_item *item = find_item(name);
    if(item == NULL) {
        return 0.0;
    }
    return item->f;
}

int conf_bool(const char *name) {
    conf_item *item = find_item(name);
    if(item == NULL) {
        return 0;
    }
    return item->i ? 1 : 0;
}

const char *conf_string(const char *name) {
    conf_item *item = find_item(name);
    if(item == NULL) {
        return NULL;
    }
    return item->s;
}

void conf_setint(const char *name, int val) {
    conf_item *item = find_item(name);
    if(item != NULL) {
        item->i = val;
    }
}

void conf_setfloat(const char *name, double val) {
    conf_item *item = find_item(name);
    if(item != NULL) {
        item->f = val;
    }
}

void conf_setbool(const char *name, int val) {
    conf_item *item = find_item(name);
    if(item != NULL) {
        item->i = val ? 1 : 0;
    }
}

void conf_setstring(const char *name, const char *val) {
    conf_item *item = find_item(name);
    char *copy = NULL;
    if(item == NULL) {
        return;
    }
    if(val != NULL) {
        copy = dup_string(val);
        if(copy == NULL) {
            return;
        }
    }
    if(item->own_s) {
        free(item->s);
    }
    item->s = copy;
    item->own_s = copy != NULL;
}

void conf_close(void) {
    unsigned int i;
    if(!ready) {
        return;
    }
    for(i = 0; i < vector_size(&items); i++) {
        conf_item *item = vector_get(&items, i);
        if(item->own_s) {
            free(item->s);
        }
    }
    vector_free(&items);
    ready = 0;
}
