#ifndef STR_H
#define STR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#define STR_STACK_SIZE (16)

typedef struct str {
    size_t len;
    char *data;
    char small[STR_STACK_SIZE];
} str;

/**
 * @brief Create an empty string object.
 * @details Creates a new, empty string object. After creating the object, it must be freed
 *          using the str_free.
 * @param dst Target string object
 */
void str_create(str *dst);

/**
 * @brief Create a new string from existing string.
 * @details Source string contents will be copied.
 * @param dst Target string object
 * @param src Source string to copy
 */
void str_from(str *dst, const str *src);

/**
 * @brief Create a string object from C string (null terminated buffer).
 * @details Source string contents will be copied.
 * @param dst Target string object
 * @param src Source C string to copy
 */
void str_from_c(str *dst, const char *src);

/**
 * @brief Create a string object from memory buffer.
 * @details Source buffer content will be copied. Please make sure that the source
 *          buffer contains no null characters, and is actual printable text string.
 * @param dst Target string object
 * @param buf Source buffer object
 * @param len Source buffer object length (bytes)
 */
void str_from_buf(str *dst, const char *buf, size_t len);

/**
 * Create a string object from file contents
 * @details Full source file content will be read, whether it contains null characters or not.
 * @param dst Target string buffer
 * @param file_name Filename to read
 */
void str_from_file(str *dst, const char *file_name);

/**
 * @brief Create a string object by format string
 * @details This uses snprintf internally, so usual printf format strings are valid.
 * @param dst Target string object
 * @param format Format string
 * @param ... Arguments list
 */
void str_from_format(str *dst, const char *format, ...);

/**
 * @brief Create a string object from range slice
 * @details This will copy a text range from another string object. Note that source string
 *          must not be same as target string!
 * @param dst Target string object
 * @param src Source string object
 * @param start Slice start position
 * @param end Slice end position
 */
void str_from_slice(str *dst, const str *src, size_t start, size_t end);

/**
 * @brief Formats a string into an existing str
 * @details This uses snprintf internally, so usual printf format strings are valid.
 * @param dst Target string object
 * @param format Format string
 * @param ... Arguments list
 */

void str_format(str *dst, const char *format, ...);

/**
 * @brief Cut away amount of characters from the end of the string.
 * @details If cut amount is larger than the length of the string, the whole string will be cleared.
 * @param dst String to cut
 * @param len Amount to cut
 */
void str_cut(str *dst, size_t len);

/**
 * @brief Reduce the string length to max_len if it exceeds it.
 * @param dst The string to truncate.
 * @param max_len The new maximum length of the string.
 */
void str_truncate(str *dst, size_t max_len);

/**
 * @brief Free string object
 * @details Frees up any memory used by the string object. Usage of the string after freeing it
 *          is undefined behaviour.
 *
 * @param string String to free
 */
void str_free(str *string);

// String modification

/**
 * @brief Change all letters to uppercase.
 */
void str_toupper(str *dst);

/**
 * @brief Change all lettes to lowercase.
 */
void str_tolower(str *dst);

/**
 * @brief Strip whitespace characters on the right side of a string.
 */
void str_rstrip(str *dst);

/**
 * @brief Strip whitespace characters on the left side of a string.
 */
void str_lstrip(str *dst);

/**
 * @brief Strip whitespace characters on both sides of a string.
 */
void str_strip(str *dst);

/**
 * @brief Append a string to another string.
 * @details Source string content will be copied.
 */
void str_append(str *dst, const str *src);

/**
 * @brief Append a C string (null terminated) to a string object.
 * @details Source string content will be copied.
 */
void str_append_c(str *dst, const char *src);

/**
 * @brief Append a C buffer (NOT null terminated) to a string object.
 * @details Source buffer content will be copied. Please make sure that the source
 *          buffer contains no null characters, and is actual printable text string.
 */
void str_append_buf(str *dst, const char *buf, size_t len);

/**
 * @brief Replace content in string with something else.
 * @details Replace occurrences of the search string with the replacement string.
 *          Limit parameter can be used to limit replacement to a certain number
 *          of occurrences if so wanted (or -1 for unlimited).
 * @param dst Target string to modify
 * @param seek String to search and replace
 * @param replacement The replacement value
 * @param limit Number of replacements performed (-1 for unlimited).
 */
void str_replace(str *dst, const char *seek, const char *replacement, int limit);

/**
 * @brief Get string length.
 */
size_t str_size(const str *string);

/**
 * @brief Find next occurrence of a character in a given string. Search is continued from given position.
 */
bool str_find_next(const str *string, char find, size_t *pos);

/**
 * @brief Find first occurrence of a character in a given string.
 */
bool str_first_of(const str *string, char find, size_t *pos);

/**
 * @brief Find last occurrence of a character in a given string.
 */
bool str_last_of(const str *string, char find, size_t *pos);

/**
 * @brief Check if strings match
 */
bool str_equal(const str *a, const str *b);

/**
 * @brief Check if strings match (match with C null terminated string)
 */
bool str_equal_c(const str *a, const char *b);

/**
 * @brief Check if strings match (match with C buffer)
 */
bool str_equal_buf(const str *a, const char *buf, size_t len);

/**
 * @brief Get a character at given index.
 */
char str_at(const str *string, size_t pos);

/**
 * @brief Delete a character at the given index.
 * @return true If a character was deleted.
 */
bool str_delete_at(str *string, size_t pos);

/**
 * @brief Set a character at given index.
 */
bool str_set_at(str *string, size_t pos, char value);

/**
 * @brief Insert a character at given index.
 */
bool str_insert_at(str *string, size_t pos, char value);

/**
 * @brief Convert string to float
 * @details Returns true if conversion was a success, false if not.
 * @param string String to convert
 * @param m Result value
 * @return true If success
 * @return false If failure (result value is invalid)
 */
bool str_to_float(const str *string, float *m);

/**
 * @brief Convert string to long
 * @details Returns true if conversion was a success, false if not.
 * @param string String to convert
 * @param m Result value
 * @return true If success
 * @return false If failure (result value is invalid)
 */
bool str_to_long(const str *string, long *m);

/**
 * @brief Convert string to int
 * @details Returns true if conversion was a success, false if not.
 * @param string String to convert
 * @param m Result value
 * @return true If success
 * @return false If failure (result value is invalid)
 */
bool str_to_int(const str *string, int *m);

/**
 * @brief Returns a C string compatible represantation of a string object.
 * @details Note! This will become invalid if any changes are performanced on the string
 *          object. Use with care!
 * @param src String to convert
 * @return const char*
 */
const char *str_c(const str *src);

#endif // STR_H
