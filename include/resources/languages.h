#ifndef LANGUAGES_H
#define LANGUAGES_H

/*
 * This file should handle loading language file(s)
 * and support getting text. Maybe some function to rendering text index on
 * texture with the help of text.h ?
 */

int lang_init();
void lang_close();

// Maybe something like this ?
const char *lang_get(unsigned int id);

#endif // LANGUAGES_H
