#ifndef _UTIL_STRING_H
#define _UTIL_STRING_H

#include <stddef.h>

typedef struct str_t {
    size_t len;
    char* data;
} str;

void str_create(str *string);
void str_create_from_cstr(str *string, const char *cstr);
void str_create_from_data(str *string, const char *data, size_t len);
void str_free(str *string);

size_t str_size(str *string);

void str_substr(str *dst, str *src, size_t start, size_t end);
void str_copy(str *dst, str *src);
void str_append(str *dst, str *src);
void str_append_c(str *dst, const char *src);
void str_prepend(str *dst, str *src);

int str_first_of(str *string, char find, size_t *pos);
int str_next_of(str *string, char find, size_t *pos);
int str_last_of(str *string, char find, size_t *pos);

int str_equal(str *string, str *string_b);

void str_toupper(str *string);
void str_tolower(str *string);

char str_at(str *string, size_t pos);
int str_cmp(str *string, str *string_b);

int str_to_int(str *string, int *result);
int str_to_long(str *string, long *result);
int str_to_float(str *string, float *result);
const char* str_c(str *string);

#endif // _UTIL_STRING_H
