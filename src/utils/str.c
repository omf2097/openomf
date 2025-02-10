#include "utils/str.h"
#include "utils/allocator.h"
#include "utils/compat.h"
#include "utils/io.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_STACK_SIZE sizeof(str)
// NOTE: fbstring has 2 flag bits so they can signal their "large" (>255 byte)
// string copy-on-write semantics. Hopefully we don't need that optimization.
#define STR_FLAGBIT_COUNT 1

// ------------------------ Memory Layout ------------------------

// here are both str string representations, "normal" and then "small":
//+------------+------------+------------+------------+------------+-----------+
//| char *data              | size_t size             | capacity               |
//+------------+------------+------------+------------+------------+-----------+
//| char small[]                                                   | spare     |
//+------------+------------+------------+------------+------------+-----------+
//
// `spare` is shorthand for referring to the last index of the small array, and
// will become the null terminating byte when the small string is at capacity.
// In a small string, `spare` counts the unused bytes.. the spare capacity.
//
// There is a flag bit to signal small/normal at the very end of the structure,
// overlapping both capacity and spare. Because small strings must have a null
// byte as their final byte for termination, a flag bit of 0 signals "small,"
// and a bit of 1 signals "large."
//
// Since the flag bits are stored in the last bits of capacity/spare, they are
// the most significant bits on little-endian architectures and least signif.
// on big-endian.

// ------------------------ Basic Accessors ------------------------

// access the 'spare' field described in the above memory layout.
// Do not add new calls to this function even on known-small strings, as
// seperating the flag bits from the spare capacity is finicky work.
//
// Use instead: str_issmall() or str_size().
static inline uint8_t str_smallspare(str const *s) {
    uint8_t spare;
    memcpy(&spare, &s->small[STR_STACK_SIZE - 1], 1);
    return spare;
}

// access the heap-allocated capacity of a "normal" string.
// meaningless for small strings, which always have a capacity of STR_STACK_SIZE.
static inline size_t str_normalcapacity(str const *s) {
#ifdef BIG_ENDIAN_BUILD
    // last bit (LSB) of capacity contains flag bit. mask it off.
    return s->normal.capacity >> STR_FLAGBIT_COUNT << STR_FLAGBIT_COUNT;
#else
    // last bit (MSB) of capacity contains flag bit. shift it off.
    return s->normal.capacity << STR_FLAGBIT_COUNT;
#endif
}

// Sets the capacity and flag bits of a normal (heap-allocated) string.
static inline void str_normalsetcapacity(str *s, size_t capacity) {
    // capacity's least significant bits must be zero, so we can repurpose them
    // as flag bits. This is guaranteed by str_roundupcapacity.
    assert((capacity & ((1 << STR_FLAGBIT_COUNT) - 1)) == 0);
#ifdef BIG_ENDIAN_BUILD
    // use last bit (LSB) of capacity as flag bit
    size_t flag = 1;
    s->normal.capacity = capacity | flag;
#else
    // use last bit (MSB) of capacity as flag bit
    size_t flag = SIZE_MAX ^ (SIZE_MAX >> 1);
    // shift LSB of input capacity off, as we know its never set.
    s->normal.capacity = (capacity >> 1) | flag;
#endif
}

// Sets the size (a la str_size) and flag bits of a small string.
static inline void str_smallsetsize(str *s, uint8_t size) {
    assert(size < STR_STACK_SIZE);
    uint8_t spare = STR_STACK_SIZE - 1 - size;
#ifdef BIG_ENDIAN_BUILD
    // shift spare over to create zero flag bits on the end of spare.
    spare <<= STR_FLAGBIT_COUNT;
#else
    // no-op, flag bits are MSB on little-endian; and are already zeroed.
#endif
    memcpy(&s->small[STR_STACK_SIZE - 1], &spare, 1);
}

static bool str_issmall(str const *s) {
    // last bit of capacity contains flag bits
    // 0 is "small", 1 is "normal"
#ifdef BIG_ENDIAN_BUILD
    // last bit is least significant
    return (str_smallspare(s) & 0x01) == 0;
#else
    // last bit is most significant
    return (str_smallspare(s) & 0x80) == 0;
#endif
}

size_t str_size(str const *s) {
    uint8_t spare = str_smallspare(s);
#ifdef BIG_ENDIAN_BUILD
    // shift away the flag bits.
    spare >>= STR_FLAGBIT_COUNT;
#else
    // no-op on little endian, because the flag bits (MSB) are zero for small strings.
#endif
    return str_issmall(s) ? (STR_STACK_SIZE - 1 - spare) : s->normal.size;
}

static inline char *str_ptr(str *s) {
    return str_issmall(s) ? s->small : s->normal.data;
}

char const *str_c(str const *s) {
    return str_issmall(s) ? s->small : s->normal.data;
}

static inline void str_zero(str *s) {
    str_ptr(s)[str_size(s)] = '\0';
}

static inline size_t str_roundupcapacity(size_t capacity) {
    // we could set granularity even larger, if we wanted to realloc less often (& use more memory).
    size_t const granularity = 1 << STR_FLAGBIT_COUNT;
    // round up to nearest multiple of granularity (a power of 2)
    return (capacity + (granularity - 1)) & ~(granularity - 1);
}

// ------------------------ Create & destroy ------------------------

// destructively resizes str to size
// returns a char * which the caller must write size non-null bytes to, followed by a null byte.
static char *str_resize_buffer(str *s, size_t size) {
    size_t size_with_zero = size + 1;
    bool become_small = size_with_zero <= STR_STACK_SIZE;
    bool is_small = str_issmall(s);
    if(become_small && is_small) {
        str_smallsetsize(s, size);
        return s->small;
    } else if(become_small && !is_small) {
        omf_free(s->normal.data);
        str_smallsetsize(s, size);
        return s->small;
    } else if(!become_small && is_small) {
        size_t capacity = str_roundupcapacity(size_with_zero);
        str_normalsetcapacity(s, capacity);
        s->normal.data = omf_malloc(capacity);
        s->normal.size = size;
        return s->normal.data;
    } else /* if(!become_small && !is_small) */ {
        if(size_with_zero > str_normalcapacity(s)) {
            size_t capacity = str_roundupcapacity(size_with_zero);
            str_normalsetcapacity(s, capacity);
            s->normal.data = omf_realloc(s->normal.data, capacity);
        }
        s->normal.size = size;
        return s->normal.data;
    }
}

// resizes str to size while preserving the contents (or as much will fit)
static void str_resize_and_copy_buffer(str *s, size_t size) {
    size_t size_with_zero = size + 1;
    bool become_small = size_with_zero <= STR_STACK_SIZE;
    bool is_small = str_issmall(s);
    if(become_small && is_small) {
        str_smallsetsize(s, size);
    } else if(become_small && !is_small) {
        char *old_data = s->normal.data;
        memcpy(s->small, old_data, size);
        str_smallsetsize(s, size);
        omf_free(old_data);
    } else if(!become_small && is_small) {
        // can't write to s until we've copied s->small to the heap
        size_t capacity = str_roundupcapacity(size_with_zero);
        char *data = omf_malloc(capacity);
        memcpy(data, s->small, STR_STACK_SIZE);
        s->normal.data = data;
        str_normalsetcapacity(s, capacity);
        s->normal.size = size;
    } else /* if (!become_small && !is_small) */ {
        if(size_with_zero > str_normalcapacity(s)) {
            size_t capacity = str_roundupcapacity(size_with_zero);
            str_normalsetcapacity(s, capacity);
            s->normal.data = omf_realloc(s->normal.data, capacity);
        }
        s->normal.size = size;
    }
}

void str_create(str *s) {
    memset(s, 0, sizeof(str));
    str_smallsetsize(s, 0);
}

void str_from(str *dst, const str *src) {
    str_from_buf(dst, str_c(src), str_size(src));
}

void str_from_buf(str *dst, const char *buf, size_t len) {
    str_create(dst);
    memcpy(str_resize_buffer(dst, len), buf, len);
    str_zero(dst);
}

void str_from_file(str *dst, const char *file_name) {
    FILE *handle = file_open(file_name, "rb");
    long size = file_size(handle);
    str_create(dst);
    file_read(handle, str_resize_buffer(dst, size), size);
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
        log_error("Call to vsnprintf returned -1");
        abort();
    }

    // Make sure there is enough room for our vsnprintf call plus ending NULL,
    // then render the output to our new buffer.
    if((int)str_size(dst) < size) {
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
        log_error("Call to vsnprintf returned -1");
        abort();
    }

    // Make sure there is enough room for our vsnprintf call plus ending NULL,
    // then render the output to our new buffer.
    str_create(dst);
    vsnprintf(str_resize_buffer(dst, size), size + 1, format, args2);
    va_end(args2);
}

void str_from_slice(str *dst, const str *src, size_t start, size_t end) {
    assert(dst != src);
    assert(start < end);
    size_t src_len = str_size(src);
    if(end > src_len)
        end = src_len;
    if(start > end)
        start = end;
    size_t len = end - start;
    str_from_buf(dst, str_c(src) + start, len);
}

void str_free(str *dst) {
    if(dst == NULL) {
        return;
    }
    if(!str_issmall(dst)) {
        omf_free(dst->normal.data);
    }
    memset(dst, 0, sizeof(str));
}

// ------------------------ Modification ------------------------

void str_toupper(str *dst) {
    char *ptr = str_ptr(dst);
    size_t len = str_size(dst);
    for(size_t i = 0; i < len; i++) {
        ptr[i] = toupper(ptr[i]);
    }
}

void str_tolower(str *dst) {
    char *ptr = str_ptr(dst);
    size_t len = str_size(dst);
    for(size_t i = 0; i < len; i++) {
        ptr[i] = tolower(ptr[i]);
    }
}

static size_t _strip_size(const str *src, bool left) {
    size_t len = str_size(src);
    if(len == 0) {
        return 0;
    }
    const char *ptr = str_c(src);
    for(size_t i = 0; i < len; i++) {
        size_t pos = left ? i : len - i - 1;
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
    size_t len = str_size(dst);
    memmove(ptr, ptr + skip, len - skip);
    str_resize_and_copy_buffer(dst, len - skip);
    str_zero(dst);
}

void str_strip(str *dst) {
    str_rstrip(dst);
    str_lstrip(dst);
}

void str_append(str *dst, const str *src) {
    assert(dst != src);
    str_append_buf(dst, str_c(src), str_size(src));
}

void str_append_buf(str *dst, const char *buf, size_t len) {
    size_t offset = str_size(dst);
    str_resize_and_copy_buffer(dst, offset + len);
    memcpy(str_ptr(dst) + offset, buf, len);
    str_zero(dst);
}

bool str_find_next(const str *string, char find, size_t *pos) {
    const char *ptr = str_c(string);
    size_t len = str_size(string);
    for(size_t i = *pos; i < len; i++) {
        if(ptr[i] == find) {
            *pos = i;
            return true;
        }
    }
    return false;
}

void str_cut(str *dst, size_t len) {
    size_t dst_len = str_size(dst);
    if(len > dst_len)
        len = dst_len;
    dst_len -= len;
    str_resize_and_copy_buffer(dst, dst_len);
    str_zero(dst);
}

void str_truncate(str *dst, size_t max_len) {
    size_t old_len = str_size(dst);
    if(old_len > max_len) {
        str_resize_and_copy_buffer(dst, max_len);
        str_zero(dst);
    }
}

void str_replace(str *dst, const char *seek, const char *replacement, int limit) {
    size_t seek_len = strlen(seek);
    size_t replacement_len = strlen(replacement);
    assert(seek_len > 0);
    int found = 0;
    ptrdiff_t diff = replacement_len - (ptrdiff_t)seek_len;
    size_t current_pos = 0;
    size_t len = str_size(dst);
    while(str_find_next(dst, seek[0], &current_pos) && (found < limit || limit < 0)) {
        if(strncmp(str_ptr(dst) + current_pos, seek, seek_len) == 0) {
            if(diff > 0) { // Grow first, before move.
                len += diff;
                str_resize_and_copy_buffer(dst, len);
            }
            char *ptr = str_ptr(dst) + current_pos;
            memmove(ptr + replacement_len, ptr + seek_len, len - current_pos - max2(seek_len, replacement_len));
            memcpy(ptr, replacement, replacement_len);

            if(diff < 0) { // defer actual resize til after the loop
                len += diff;
            }

            found++;
            current_pos += replacement_len;
        } else {
            current_pos++;
        }
    }
    if(diff < 0 && found) { // Reduce after all is done.
        str_resize_and_copy_buffer(dst, len);
    }
    str_zero(dst);
}

// ------------------------ Getters ------------------------

bool str_first_of(const str *string, char find, size_t *pos) {
    const char *ptr = str_c(string);
    size_t len = str_size(string);
    for(size_t i = 0; i < len; i++) {
        if(ptr[i] == find) {
            *pos = i;
            return true;
        }
    }
    return false;
}

bool str_last_of(const str *string, char find, size_t *pos) {
    const char *ptr = str_c(string);
    size_t len = str_size(string);
    for(size_t i = 0; i < len; i++) {
        size_t tmp = len - i - 1;
        if(ptr[tmp] == find) {
            *pos = tmp;
            return true;
        }
    }
    return false;
}

bool str_equal(const str *a, const str *b) {
    size_t len = str_size(a);
    if(len != str_size(b)) {
        return false;
    }
    const char *ptr_a = str_c(a);
    const char *ptr_b = str_c(b);
    if(memcmp(ptr_a, ptr_b, len) != 0) {
        return false;
    }
    return true;
}

bool str_equal_buf(const str *a, const char *buf, size_t len) {
    if(str_size(a) != len) {
        return false;
    }
    const char *ptr = str_c(a);
    if(memcmp(ptr, buf, len) != 0) {
        return false;
    }
    return true;
}

char str_at(const str *string, size_t pos) {
    if(pos >= str_size(string)) {
        return '\0';
    }
    const char *ptr = str_c(string);
    return ptr[pos];
}

bool str_delete_at(str *string, size_t pos) {
    size_t len = str_size(string);
    if(pos >= len) {
        return false;
    }
    char *buf = str_ptr(string);
    size_t n = len - pos - 1;
    memmove(&buf[pos], &buf[pos + 1], n);
    len--;
    str_resize_and_copy_buffer(string, len);
    str_zero(string);
    return true;
}

bool str_set_at(str *string, size_t pos, char value) {
    if(pos >= str_size(string)) {
        return false;
    }
    char *buf = str_ptr(string);
    buf[pos] = value;
    return true;
}

bool str_insert_at(str *string, size_t pos, char value) {
    size_t len = str_size(string);
    if(pos > len) {
        return false;
    }
    str_resize_and_copy_buffer(string, len + 1);
    str_zero(string);
    char *buf = str_ptr(string);
    size_t n = len - pos;
    memmove(&buf[pos + 1], &buf[pos], n);
    buf[pos] = value;
    return true;
}

bool str_insert_buf_at(str *dst, size_t pos, const char *src, size_t src_len) {
    size_t len = str_size(dst);
    if(pos > len) {
        return false;
    }
    size_t n = len - pos;
    str_resize_and_copy_buffer(dst, len + src_len);
    str_zero(dst);
    char *buf = str_ptr(dst);
    memmove(&buf[pos + src_len], &buf[pos], n);
    memcpy(&buf[pos], src, src_len);
    return true;
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
