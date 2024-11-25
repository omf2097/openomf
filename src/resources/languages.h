#ifndef LANGUAGES_H
#define LANGUAGES_H

#include "resources/generated_languages.h" // for Lang enum
#include <stdbool.h>

/*
 * This file should handle loading language file(s)
 * and support getting text. Maybe some function to rendering text index on
 * texture with the help of text.h ?
 */

bool lang_init(void);
void lang_close(void);

enum
{
    // OMFv2.1 (Epic Challenge Arena) ships 1013 strings.
    // (latest official ENGLISH.DAT has this many strings)
    LANG_STR_COUNT = 1013,

    // OMF versions before 2.1 shipped 990 strings.
    // (GERMAN.DAT, and old ENGLISH.DAT have this many strings)
    OLD_LANG_STR_COUNT = 990,
};

// Gets an openomf localization string
const char *lang_get(unsigned int id);

// Gets an openomf localization string (from one of the tables)
#define lang_get_offset(id, offset) lang_get_offset_impl(id, id##_LAST, (offset))
const char *lang_get_offset_impl(unsigned int id, unsigned int id_last, unsigned int offset);

#endif // LANGUAGES_H
