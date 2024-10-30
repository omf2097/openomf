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

// ------------------------ Create & destroy ------------------------

static void str_resize_buffer(str *dst, size_t size) {
    size_t size_with_zero = size + 1;
    if(size_with_zero > STR_STACK_SIZE) {
        dst->data = omf_realloc(dst->data, size_with_zero);
        dst->small[0] = 0;
    } else {
        if(dst->data != NULL) {
            omf_free(dst->data);
        }
        dst->small[size] = 0;
    }
    dst->len = size;
}

static void str_resize_and_copy_buffer(str *dst, size_t size) {
    size_t size_with_zero = size + 1;
    if(size_with_zero > STR_STACK_SIZE) {
        // New size is larger than the stack buffer; do malloc.
        if(dst->data == NULL) {
            // Old string is in stack, move to heap
            dst->data = omf_malloc(size_with_zero);
            memcpy(dst->data, dst->small, dst->len);
            dst->data[dst->len] = 0;
            dst->small[0] = 0;
        } else {
            // Old string is already in heap, keep it there.
            dst->data = omf_realloc(dst->data, size_with_zero);
        }
    } else {
        // New size is small enough to move back to stack.
        if(dst->data != NULL) {
            // Old string is in heap, move to stack.
            memcpy(dst->small, dst->data, size);
            omf_free(dst->data);
            dst->small[size] = 0;
        }
    }
    dst->len = size;
}

static char *str_ptr(str *src) {
    if(src->data != NULL) {
        return src->data;
    }
    return src->small;
}

static void str_zero(str *dst) {
    if(dst->data == NULL) {
        dst->small[dst->len] = 0;
    } else {
        dst->data[dst->len] = 0;
    }
}

void str_create(str *dst) {
    memset(dst, 0, sizeof(str));
}

void str_from(str *dst, const str *src) {
    str_from_buf(dst, str_c(src), src->len);
}

void str_from_c(str *dst, const char *src) {
    str_from_buf(dst, src, strlen(src));
}

void str_from_buf(str *dst, const char *buf, size_t len) {
    str_create(dst);
    str_resize_buffer(dst, len);
    memcpy(str_ptr(dst), buf, len);
    str_zero(dst);
}

void str_from_file(str *dst, const char *file_name) {
    FILE *handle = file_open(file_name, "rb");
    long size = file_size(handle);
    str_create(dst);
    str_resize_buffer(dst, size);
    file_read(handle, str_ptr(dst), size);
    file_close(handle);
    str_zero(dst);
}

void str_format(str *dst, const char *format, ...) {
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
    if((int)dst->len < size) {
        str_resize_buffer(dst, size);
    }
    vsnprintf(str_ptr(dst), size + 1, format, args2);
    va_end(args2);
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
    str_create(dst);
    str_resize_buffer(dst, size);
    vsnprintf(str_ptr(dst), size + 1, format, args2);
    va_end(args2);
}

void str_from_slice(str *dst, const str *src, size_t start, size_t end) {
    assert(dst != src);
    assert(start < end);
    if(end > src->len)
        end = src->len;
    size_t len = end - start;
    str_create(dst);
    str_resize_buffer(dst, len);
    memcpy(str_ptr(dst), str_c(src) + start, len);
    str_zero(dst);
}

void str_free(str *dst) {
    if(dst == NULL) {
        return;
    }
    if(dst->data != NULL) {
        omf_free(dst->data);
    }
    memset(dst, 0, sizeof(str));
}

// ------------------------ Modification ------------------------

void str_toupper(str *dst) {
    char *ptr = str_ptr(dst);
    for(size_t i = 0; i < dst->len; i++) {
        ptr[i] = toupper(ptr[i]);
    }
}

void str_tolower(str *dst) {
    char *ptr = str_ptr(dst);
    for(size_t i = 0; i < dst->len; i++) {
        ptr[i] = tolower(ptr[i]);
    }
}

static size_t _strip_size(const str *src, bool left) {
    if(src->len == 0) {
        return 0;
    }
    size_t pos;
    const char *ptr = str_c(src);
    for(size_t i = 0; i < src->len; i++) {
        pos = left ? i : src->len - i - 1;
        if(!isspace(ptr[pos])) {
            return pos;
        }
    }
    return 0;
}

void str_rstrip(str *dst) {
    // This is simple, just reduce size and set ending 0.
    size_t skip = _strip_size(dst, false);
    str_resize_and_copy_buffer(dst, skip + 1);
    str_zero(dst);
}

void str_lstrip(str *dst) {
    // More complex. Move data first (memmmove!), then reduce size.
    size_t skip = _strip_size(dst, true);
    char *ptr = str_ptr(dst);
    memmove(ptr, ptr + skip, dst->len - skip);
    str_resize_and_copy_buffer(dst, dst->len - skip);
    str_zero(dst);
}

void str_strip(str *dst) {
    str_rstrip(dst);
    str_lstrip(dst);
}

void str_append(str *dst, const str *src) {
    assert(dst != src);
    str_append_buf(dst, str_c(src), src->len);
}

void str_append_c(str *dst, const char *src) {
    str_append_buf(dst, src, strlen(src));
}

void str_append_buf(str *dst, const char *buf, size_t len) {
    size_t offset = dst->len;
    str_resize_and_copy_buffer(dst, dst->len + len);
    memcpy(str_ptr(dst) + offset, buf, len);
    str_zero(dst);
}

bool str_find_next(const str *string, char find, size_t *pos) {
    const char *ptr = str_c(string);
    for(size_t i = *pos; i < string->len; i++) {
        if(ptr[i] == find) {
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
    str_resize_and_copy_buffer(dst, dst->len);
    str_zero(dst);
}

void str_replace(str *dst, const char *seek, const char *replacement, int limit) {
    size_t seek_len = strlen(seek);
    size_t replacement_len = strlen(replacement);
    assert(seek_len > 0);
    int found = 0;
    ptrdiff_t diff = replacement_len - (ptrdiff_t)seek_len;
    size_t current_pos = 0;
    while(str_find_next(dst, seek[0], &current_pos) && (found < limit || limit < 0)) {
        if(strncmp(str_ptr(dst) + current_pos, seek, seek_len) == 0) {
            if(diff > 0) { // Grow first, before move.
                str_resize_and_copy_buffer(dst, dst->len + diff);
            }
            char *ptr = str_ptr(dst);
            memmove(ptr + current_pos + replacement_len, ptr + current_pos + seek_len,
                    dst->len - current_pos - max2(replacement_len, seek_len));
            memcpy(ptr + current_pos, replacement, replacement_len);
            if(diff < 0) { // Reduce after all is done.
                str_resize_and_copy_buffer(dst, dst->len + diff);
            }
            str_zero(dst);

            found++;
            current_pos += replacement_len;
        } else {
            current_pos++;
        }
    }
}

// ------------------------ Getters ------------------------

size_t str_size(const str *string) {
    return string->len;
}

bool str_first_of(const str *string, char find, size_t *pos) {
    const char *ptr = str_c(string);
    for(size_t i = 0; i < string->len; i++) {
        if(ptr[i] == find) {
            *pos = i;
            return true;
        }
    }
    return false;
}

bool str_last_of(const str *string, char find, size_t *pos) {
    size_t tmp;
    const char *ptr = str_c(string);
    for(size_t i = 0; i < string->len; i++) {
        tmp = string->len - i - 1;
        if(ptr[tmp] == find) {
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
    const char *ptr_a = str_c(a);
    const char *ptr_b = str_c(b);
    if(strncmp(ptr_a, ptr_b, a->len) != 0) {
        return false;
    }
    return true;
}

bool str_equal_c(const str *a, const char *b) {
    if(a->len != strlen(b)) {
        return false;
    }
    const char *ptr = str_c(a);
    if(strncmp(ptr, b, a->len) != 0) {
        return false;
    }
    return true;
}

bool str_equal_buf(const str *a, const char *buf, size_t len) {
    if(a->len != len) {
        return false;
    }
    const char *ptr = str_c(a);
    if(strncmp(ptr, buf, a->len) != 0) {
        return false;
    }
    return true;
}

char str_at(const str *string, size_t pos) {
    if(pos >= str_size(string)) {
        return 0;
    }
    const char *ptr = str_c(string);
    return ptr[pos];
}

// ------------------------ Type conversion ------------------------

bool str_to_float(const str *string, float *result) {
    char *end;
    const char *ptr = str_c(string);
    *result = strtof(ptr, &end);
    return (ptr != end);
}

bool str_to_long(const str *string, long *result) {
    char *end;
    const char *ptr = str_c(string);
    *result = strtol(ptr, &end, 10);
    return (ptr != end);
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
    return string->data ? string->data : string->small;
}
