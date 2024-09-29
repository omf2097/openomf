#include "utils/str.h"
#include "utils/allocator.h"
#include "utils/io.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_BLOCK_SIZE (1024)

#define STR_ALLOC(string, size)                                                                                        \
    do {                                                                                                               \
        string->len = (size);                                                                                          \
        string->data = omf_calloc(string->len + 1, 1);                                                                 \
    } while(0)

#define STR_REALLOC(string, size)                                                                                      \
    do {                                                                                                               \
        string->len = (size);                                                                                          \
        string->data = omf_realloc(string->data, string->len + 1);                                                     \
    } while(0)

#define STR_ZERO(string) string->data[string->len] = 0

// ------------------------ Create & destroy ------------------------

void str_create(str *dst) {
    STR_ALLOC(dst, 0);
}

void str_from(str *dst, const str *src) {
    str_from_buf(dst, src->data, src->len);
}

void str_from_c(str *dst, const char *src) {
    str_from_buf(dst, src, strlen(src));
}

void str_from_buf(str *dst, const char *buf, size_t len) {
    STR_ALLOC(dst, len);
    memcpy(dst->data, buf, len);
    STR_ZERO(dst);
}

void str_from_file(str *dst, const char *file_name) {
    FILE *handle = file_open(file_name, "rb");
    long size = file_size(handle);
    STR_ALLOC(dst, size + 1);
    file_read(handle, dst->data, size);
    file_close(handle);
    STR_ZERO(dst);
}

void str_from_format(str *dst, const char *format, ...) {
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
        abort();
    }

    // Make sure there is enough room for our vsnprintf call plus ending NULL,
    // then render the output to our new buffer.
    STR_ALLOC(dst, size);
    vsnprintf(dst->data, size + 1, format, args2);
    va_end(args2);
}

void str_from_slice(str *dst, const str *src, size_t start, size_t end) {
    assert(dst != src);
    assert(start < end);
    if(end > src->len)
        end = src->len;
    size_t len = end - start;
    STR_ALLOC(dst, len);
    memcpy(dst->data, src->data + start, len);
    STR_ZERO(dst);
}

void str_free(str *dst) {
    if(dst == NULL) {
        return;
    }
    omf_free(dst->data);
    dst->len = 0;
}

// ------------------------ Modification ------------------------

void str_toupper(str *dst) {
    for(size_t i = 0; i < dst->len; i++) {
        dst->data[i] = toupper(dst->data[i]);
    }
}

void str_tolower(str *dst) {
    for(size_t i = 0; i < dst->len; i++) {
        dst->data[i] = tolower(dst->data[i]);
    }
}

static size_t _strip_size(const str *src, bool left) {
    if(src->len == 0) {
        return 0;
    }
    size_t pos;
    for(size_t i = 0; i < src->len; i++) {
        pos = left ? i : src->len - i - 1;
        if(!isspace(src->data[pos])) {
            return pos;
        }
    }
    return 0;
}

void str_rstrip(str *dst) {
    // This is simple, just reduce size and set ending 0.
    size_t skip = _strip_size(dst, false);
    STR_REALLOC(dst, skip + 1);
    STR_ZERO(dst);
}

void str_lstrip(str *dst) {
    // More complex. Move data first (memmmove!), then reduce size.
    size_t skip = _strip_size(dst, true);
    memmove(dst->data, dst->data + skip, dst->len - skip);
    STR_REALLOC(dst, dst->len - skip);
    STR_ZERO(dst);
}

void str_strip(str *dst) {
    str_rstrip(dst);
    str_lstrip(dst);
}

void str_append(str *dst, const str *src) {
    assert(dst != src);
    str_append_buf(dst, src->data, src->len);
}

void str_append_c(str *dst, const char *src) {
    str_append_buf(dst, src, strlen(src));
}

void str_append_buf(str *dst, const char *buf, size_t len) {
    size_t offset = dst->len;
    STR_REALLOC(dst, dst->len + len);
    memcpy(dst->data + offset, buf, len);
    STR_ZERO(dst);
}

static bool _find_next(const str *string, char find, size_t *pos) {
    for(size_t i = *pos; i < string->len; i++) {
        if(string->data[i] == find) {
            *pos = i;
            return true;
        }
    }
    return false;
}

void str_cut(str *dst, size_t len) {
    if(len > dst->len)
        len = dst->len;
    dst->len -= len;
    STR_REALLOC(dst, dst->len);
    STR_ZERO(dst);
}

void str_replace(str *dst, const char *seek, const char *replacement, int limit) {
    size_t seek_len = strlen(seek);
    size_t replacement_len = strlen(replacement);
    assert(seek_len > 0);
    int found = 0;
    size_t diff = replacement_len - seek_len;
    size_t current_pos = 0;
    while(_find_next(dst, seek[0], &current_pos) && (found < limit || limit < 0)) {
        if(strncmp(dst->data + current_pos, seek, seek_len) == 0) {
            if(diff > 0) { // Grow first, before move.
                STR_REALLOC(dst, dst->len + diff);
            }
            memmove(dst->data + current_pos + replacement_len, dst->data + current_pos + seek_len,
                    dst->len - replacement_len - current_pos);
            memcpy(dst->data + current_pos, replacement, replacement_len);
            if(diff < 0) { // Reduce after all is done.
                STR_REALLOC(dst, dst->len + diff);
            }
            STR_ZERO(dst);

            found++;
        }
        current_pos++;
    }
}

// ------------------------ Getters ------------------------

size_t str_size(const str *string) {
    return string->len;
}

bool str_first_of(const str *string, char find, size_t *pos) {
    for(size_t i = 0; i < string->len; i++) {
        if(string->data[i] == find) {
            *pos = i;
            return true;
        }
    }
    return false;
}

bool str_last_of(const str *string, char find, size_t *pos) {
    size_t tmp;
    for(size_t i = 0; i < string->len; i++) {
        tmp = string->len - i - 1;
        if(string->data[tmp] == find) {
            *pos = tmp;
            return true;
        }
    }
    return false;
}

bool str_equal(const str *a, const str *b) {
    if(a->len != b->len) {
        return false;
    }
    if(strncmp(a->data, b->data, a->len) != 0) {
        return false;
    }
    return true;
}

bool str_equal_c(const str *a, const char *b) {
    if(a->len != strlen(b)) {
        return false;
    }
    if(strncmp(a->data, b, a->len) != 0) {
        return false;
    }
    return true;
}

bool str_equal_buf(const str *a, const char *buf, size_t len) {
    if(a->len != len) {
        return false;
    }
    if(strncmp(a->data, buf, a->len) != 0) {
        return false;
    }
    return true;
}

char str_at(const str *string, size_t pos) {
    if(pos >= str_size(string)) {
        return 0;
    }
    return string->data[pos];
}

// ------------------------ Type conversion ------------------------

bool str_to_float(const str *string, float *result) {
    char *end;
    *result = strtof(string->data, &end);
    return (string->data != end);
}

bool str_to_long(const str *string, long *result) {
    char *end;
    *result = strtol(string->data, &end, 10);
    return (string->data != end);
}

bool str_to_int(const str *string, int *result) {
    long value;
    bool got = str_to_long(string, &value);
    if(got) {
        *result = clamp_long_to_int(value);
    }
    return got;
}

const char *str_c(const str *string) {
    // At the moment, the internal representation of
    // string is compatible with C strings. So just return
    // a pointer to that data
    return string->data;
}
