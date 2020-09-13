#include "utils/str.h"
#include "utils/log.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

void str_create(str *string) {
    string->len = 0;
    string->data = calloc(1, 1);
}

void str_create_from_cstr(str *string, const char *cstr) {
    string->len = strlen(cstr);
    string->data = malloc(string->len + 1);
    memcpy(string->data, cstr, string->len);
    string->data[string->len] = 0;
}

void str_create_from_data(str *string, const char *data, size_t len) {
    string->len = len;
    string->data = malloc(len + 1);
    memcpy(string->data, data, string->len);
    string->data[string->len] = 0;
}

void str_free(str *string) {
    free(string->data);
    string->data = NULL;
    string->len = 0;
}

size_t str_size(const str *string) {
    return string->len;
}

void str_remove_at(str *src, size_t pos) {
   memmove(src->data + pos, src->data + pos + 1, src->len - pos - 1);
   src->len--;
}

void str_printf(str *dst, const char *format, ...) {
    int size;
    va_list args1;
    va_list args2;

    // Find size for the printf output. Make sure to copy the variadic
    // args for the next vsnprintf call.
    va_start(args1, format);
    va_copy(args2, args1);
    size = vsnprintf(NULL, 0, format, args1);
    va_end(args1);

    // vsnprintf may return -1 for errors, catch that here.
    if(size < 0) {
        PERROR("Call to vsnprintf returned -1");
        exit(1);
    }

    // Make sure there is enough room for our vsnprintf call plus ending NULL,
    // then render the output to our new buffer.
    dst->data = realloc(dst->data, dst->len + size + 1);
    vsnprintf(dst->data + dst->len, size + 1, format, args2);
    va_end(args2);

    dst->len += size;
    dst->data[dst->len] = 0;
}

void str_slice(str *dst, const str *src, size_t start, size_t end) {
    assert(start < end);
    size_t len = end - start;
    dst->data = realloc(dst->data, len + 1);
    dst->len = len;
    memcpy(dst->data, src->data + start, len);
    dst->data[dst->len] = 0;
}

void str_copy(str *dst, const str *src) {
    dst->data = realloc(dst->data, src->len + 1);
    dst->len = src->len;
    memcpy(dst->data, src->data, dst->len);
    dst->data[dst->len] = 0;
}

void str_append(str *dst, const str *src) {
    dst->data = realloc(dst->data, dst->len + src->len + 1);
    memcpy(dst->data + dst->len, src->data, src->len);
    dst->len += src->len;
    dst->data[dst->len] = 0;
}

void str_append_c(str *dst, const char *src) {
    size_t srclen = strlen(src);
    dst->data = realloc(dst->data, dst->len + srclen + 1);
    memcpy(dst->data + dst->len, src, srclen);
    dst->len += srclen;
    dst->data[dst->len] = 0;
}

void str_prepend(str *dst, const str *src) {
    dst->data = realloc(dst->data, dst->len + src->len + 1);
    memmove(dst->data + src->len, dst->data, dst->len);
    memcpy(dst->data, src->data, src->len);
    dst->len += src->len;
    dst->data[dst->len] = 0;
}

void str_prepend_c(str *dst, const char *src) {
    size_t srclen = strlen(src);
    dst->data = realloc(dst->data, dst->len + srclen + 1);
    memmove(dst->data + srclen, dst->data, dst->len);
    memcpy(dst->data, src, srclen);
    dst->len += srclen;
    dst->data[dst->len] = 0;
}

int str_first_of(const str *string, char find, size_t *pos) {
    for(size_t i = 0; i < string->len; i++) {
        if(string->data[i] == find) {
            *pos = i;
            return 1;
        }
    }
    return 0;
}

int str_next_of(const str *string, char find, size_t *pos) {
    for(size_t i = *pos; i < string->len; i++) {
        if(string->data[i] == find) {
            *pos = i;
            return 1;
        }
    }
    return 0;
}

int str_last_of(const str *string, char find, size_t *pos) {
    for(size_t i = string->len; i > 0; i--) {
        if(string->data[i-1] == find) {
            *pos = i-1;
            return 1;
        }
    }
    return 0;
}

int str_equal(const str *string, const str *string_b) {
    if(string->len != string_b->len) {
        return 0;
    }
    if(strncmp(string->data, string_b->data, string->len) != 0) {
        return 0;
    }
    return 1;
}

char str_at(const str *string, size_t pos) {
    if(pos >= str_size(string)) {
        return 0;
    }
    return string->data[pos];
}

void str_toupper(str *string) {
    for(size_t i = 0; i < str_size(string); i++) {
        string->data[i] = toupper(string->data[i]);
    }
}

void str_tolower(str *string) {
    for(size_t i = 0; i < str_size(string); i++) {
        string->data[i] = tolower(string->data[i]);
    }
}

int str_to_int(const str *string, int *result) {
    *result = atoi(str_c(string));
    return 1;
}

int str_to_float(const str *string, float *result) {
    *result = atof(str_c(string));
    return 1;
}

int str_to_long(const str *string, long *result) {
    *result = atol(str_c(string));
    return 1;
}

const char* str_c(const str *string) {
    // At the moment, the internal representation of
    // string is compatible with C strings. So just return
    // a pointer to that data
    return string->data;
}

const char* str_c_alloc(const str *src) {
    char *ptr = malloc(src->len + 1);
    memcpy(ptr, src->data, src->len + 1);
    ptr[src->len] = 0;
    return ptr;
}
