#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdarg.h>

typedef struct str_t {
    size_t len;
    char* data;
} str;

void str_create(str *string);
void str_create_from_cstr(str *string, const char *cstr);
void str_create_from_data(str *string, const char *data, size_t len);
void str_free(str *string);

size_t str_size(const str *string);
void str_slice(str *dst, const str *src, size_t start, size_t end);
void str_copy(str *dst, const str *src);
void str_append(str *dst, const str *src);
void str_append_c(str *dst, const char *src);
void str_prepend(str *dst, const str *src);
void str_prepend_c(str *dst, const char *src);
void str_remove_at(str *string, size_t pos);

void str_printf(str *dst, const char *format, ...);

int str_first_of(const str *string, char find, size_t *pos);
int str_next_of(const str *string, char find, size_t *pos);
int str_last_of(const str *string, char find, size_t *pos);

int str_equal(const str *string, const str *string_b);

void str_toupper(str *string);
void str_tolower(str *string);

char str_at(const str *string, size_t pos);

int str_to_int(const str *string, int *result);
int str_to_long(const str *string, long *result);
int str_to_float(const str *string, float *result);
const char* str_c(const str *string);
const char* str_c_alloc(const str *string);

#endif // STR_H
