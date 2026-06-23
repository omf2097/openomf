/**
 * @file sstream.h
 * @brief Read-only ASCII string stream.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SSTREAM_H
#define SSTREAM_H

#include <stdbool.h>
#include <string.h>

typedef struct sstream {
    const char *data; ///< Borrowed buffer!
    int len;          ///< Buffer length in bytes.
    int pos;          ///< Current read position.
} sstream;

/** @brief Open a stream to a buffer. */
static inline void sstream_open(sstream *s, const char *data, const int len) {
    s->data = data;
    s->len = len;
    s->pos = 0;
}

/** @brief Open a stream to a null-terminated string */
static inline void sstream_open_c(sstream *s, const char *cstr) {
    sstream_open(s, cstr, (int)strlen(cstr));
}

/** @brief Tells if the read position is at or past the end of the buffer. */
static inline bool sstream_eof(const sstream *s) {
    return s->pos >= s->len;
}

/** @brief Returns the current read position. */
static inline int sstream_pos(const sstream *s) {
    return s->pos;
}

/** @brief Returns the number of bytes left to read. */
static inline int sstream_left(const sstream *s) {
    return s->pos < s->len ? s->len - s->pos : 0;
}

/** @brief Returns a pointer to the buffer data at the current position. */
static inline const char *sstream_ptr(const sstream *s) {
    return s->data + s->pos;
}

/** @brief Returns the character at the current position, or '\0' if at the end. */
static inline char sstream_peek(const sstream *s) {
    return s->pos < s->len ? s->data[s->pos] : '\0';
}

/** @brief Returns the character at an offset from the current position., or '\0' if OOB. */
static inline char sstream_peek_at(const sstream *s, const int offset) {
    const int at = s->pos + offset;
    return (at >= 0 && at < s->len) ? s->data[at] : '\0';
}

/** @brief Returns the character at the current position and advances, or '\0' at end. */
static inline char sstream_get(sstream *s) {
    return s->pos < s->len ? s->data[s->pos++] : '\0';
}

/** @brief Advances the read position by given amount */
static inline void sstream_skip(sstream *s, const int n) {
    s->pos += n;
    if(s->pos > s->len) {
        s->pos = s->len;
    } else if(s->pos < 0) {
        s->pos = 0;
    }
}

/** @brief Reads a long at the current position. */
bool sstream_read_long(sstream *s, long *out, long min, long max);

#endif // SSTREAM_H
