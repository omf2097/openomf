/**
 * @file c_string_util.h
 * @brief C string utility functions.
 * @details Provides safer string operations and cross-platform compatibility wrappers
 *          for common string functions. These complement the standard library functions
 *          with safer alternatives that handle edge cases.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef C_STRING_UTIL_H
#define C_STRING_UTIL_H

#include <stddef.h>

/**
 * @brief Copy a string with guaranteed null-termination, truncating if necessary.
 * @details Like strncpy(), but always null-terminates the destination buffer.
 *          If the source is longer than n-1 characters, it is truncated.
 * @param dest Destination buffer
 * @param src Source string to copy
 * @param n Size of the destination buffer
 * @return Pointer to dest
 */
char *strncpy_or_truncate(char *dest, const char *src, size_t n);

/**
 * @brief Copy a string with guaranteed null-termination, aborting on overflow.
 * @details Like strncpy(), but always null-terminates and aborts if the source
 *          string would be truncated. Use when truncation is a bug.
 * @param dest Destination buffer
 * @param src Source string to copy
 * @param n Size of the destination buffer
 * @return Pointer to dest
 */
char *strncpy_or_abort(char *dest, const char *src, size_t n);

/**
 * @internal
 * @brief Internal implementation - use omf_strdup() macro instead.
 * @see omf_strdup
 */
char *omf_strdup_real(char const *s, char const *file, int line);

/**
 * @brief Duplicate a string using the OMF allocator.
 * @param s String to duplicate
 * @return Newly allocated copy of the string
 */
#define omf_strdup(s) omf_strdup_real((s), __FILE__, __LINE__)

/**
 * @internal
 * @brief Internal implementation - use omf_strndup() macro instead.
 * @see omf_strndup
 */
char *omf_strndup_real(char const *s, size_t n, char const *file, int line);

/**
 * @brief Duplicate at most n characters of a string using the OMF allocator.
 * @param s String to duplicate
 * @param n Maximum number of characters to copy
 * @return Newly allocated copy of the string (always null-terminated)
 */
#define omf_strndup(s, n) omf_strndup_real((s), (n), __FILE__, __LINE__)

/**
 * @brief Case-insensitive string comparison with length limit.
 * @details Cross-platform wrapper for strncasecmp/strnicmp.
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of characters to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int omf_strncasecmp(char const *s1, char const *s2, size_t n);

/**
 * @brief Case-insensitive string comparison.
 * @details Cross-platform wrapper for strcasecmp/stricmp.
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int omf_strcasecmp(char const *s1, char const *s2);

/**
 * @brief Get the length of a string with a maximum limit.
 * @details Reads up to strsz bytes of str, returning the position of the
 *          first null character or strsz if none was found.
 * @param str String to measure
 * @param strsz Maximum number of bytes to read
 * @return Length of string, or strsz if no null terminator found
 */
size_t omf_strnlen_s(char const *str, size_t strsz);

#endif // C_STRING_UTIL_H
