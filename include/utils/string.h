#ifndef _UTIL_STRING_H
#define _UTIL_STRING_H

#include <string.h>

typedef struct str_t {
	size_t len;
	char* data;
} str;

void str_create(str *string);
void str_create_from_cstr(str *string, const char *cstr);
void str_create_from_data(str *string, const char *data, size_t len);
void str_free(str *string);

size_t str_size(str *string);

void str_copy(str *string, str *source);
void str_append(str *string, str *source);
void str_prepend(str *string, str *source);

size_t str_first_of(str *string, char find, int *pos);
size_t str_last_of(str *string, char find, int *pos);

int str_to_int(str *string, int *result);
int str_to_long(str *string, long *result);
int str_to_float(str *string, float *result);
const char* str_c(str *string);

#endif // _UTIL_STRING_H
