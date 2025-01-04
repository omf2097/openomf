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

/**
 * @brief Gets a C string from a language string index.
 * @details Cheap to call, as it's a simple array lookup.
 *  Currently, returned strings are encoded in CP437; this may
 *  not be true in the future, there's been talk of switching to UTF-8.
 * @param id Lang id to translate, from the enum. Example: LangChooseYourPilot
 */
const char *lang_get(int id);

/**
 * @brief Translates a language string range and offset to a C String.
 * @details Cheap to call, as it's a single array lookup.
 * @param LangStr Lang id to translate, from the enum. Example: LangTrainingHelp
 * @param offset into the range, 0 returns the first of the range.
 */
#define lang_get_offset(LangStr, offset) lang_get_offset_impl(LangStr, LangStr##_Last, (offset))
const char *lang_get_offset_impl(int id, int last, int offset);

#endif // LANGUAGES_H
