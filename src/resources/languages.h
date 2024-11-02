#ifndef LANGUAGES_H
#define LANGUAGES_H

/*
 * This file should handle loading language file(s)
 * and support getting text. Maybe some function to rendering text index on
 * texture with the help of text.h ?
 */

int lang_init(void);
void lang_close(void);

// Gets an OMF 2097 localization string
const char *lang_get(unsigned int id);
// Gets an openomf localization string
const char *lang_get2(unsigned int id);

#endif // LANGUAGES_H
