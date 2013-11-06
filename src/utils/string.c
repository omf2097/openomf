#include "utils/string.h"

#include <stdlib.h>
#include <string.h>

void str_create(str *string) {
    string->len = 0;
    string->data = NULL;
}

void str_create_from_cstr(str *string, const char *cstr) {
    if (cstr) {
        string->len = strlen(cstr);
        string->data = malloc(string->len + 1);
        memcpy(string->data, cstr, string->len);
        string->data[string->len] = 0;
    } else {
        string->len = 0;
        string->data = NULL;
    }
}

void str_create_from_data(str *string, const char *data, size_t len) {
    string->len = len;
    string->data = malloc(len + 1);
    memcpy(string->data, data, len);
    string->data[string->len] = 0;
}

void str_free(str *string) {
    if (string->data) {
        free(string->data);
    }
}

size_t str_size(str *string) {
    return string->len;
}

void str_copy(str *string, str *source) {
    if (source->data) {
        string->data = realloc(string->data, source->len + 1);
        string->len = source->len;
        memcpy(string->data, source->data, string->len);
        string->data[string->len] = 0;
    } else {
        string->data = NULL;
        string->len = 0;
    }
}

void str_append(str *string, str *source) {
    string->data = realloc(string->data, string->len + source->len + 1);
    memcpy(string->data + string->len, source->data, source->len);
    string->len += source->len;
    string->data[string->len] = 0;
}

void str_prepend(str *string, str *source) {
    string->data = realloc(string->data, string->len + source->len + 1);
    memmove(string->data + source->len, string->data, string->len);
    memcpy(string->data, source->data, source->len);
    string->len += source->len;
    string->data[string->len] = 0;
}

size_t str_first_of(str *string, char find, int *pos) {
    for(int i = 0; i < string->len; i++) {
        if(string->data[i] == find) {
            *pos = i;
            return 1;
        }
    }
    return 0;
}

size_t str_last_of(str *string, char find, int *pos) {
    for(int i = string->len; i > 0; i--) {
        if(string->data[i-1] == find) {
            *pos = i;
            return 1;
        }
    }
    return 0;
}

int str_to_int(str *string, int *result) {
    *result = atoi(str_c(string));
    return 1;
}

int str_to_float(str *string, float *result) {
    *result = atof(str_c(string));
    return 1;
}

int str_to_long(str *string, long *result) {
    *result = atol(str_c(string));
    return 1;
}

const char* str_c(str *string) {
    // At the moment, the internal representation of 
    // string is compatible with C strings. So just return
    // a pointer to that data
    return string->data;
}
