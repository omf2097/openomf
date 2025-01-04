#ifndef LANGUAGES_H
#define LANGUAGES_H

#include "resources/generated_languages.h"
#include <stdbool.h>

/*
 * This file should handle loading language file(s)
 * and support getting text. Maybe some function to rendering text index on
 * texture with the help of text.h ?
 */

bool lang_init(void);
void lang_close(void);

// Gets a localization string
const char *lang_get(int id);

// Gets an openomf localization string (from one of the tables)
#define lang_get_offset(LangStr, offset) lang_get_offset_impl(LangStr, LangStr##_Last, (offset))
const char *lang_get_offset_impl(int id, int last, int offset);

#endif // LANGUAGES_H
