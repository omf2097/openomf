#ifndef _LANGUAGES_H
#define _LANGUAGES_H

/*
* This file should handle loading language file(s)
* and support getting text. Maybe some function to rendering text index on 
* texture with the help of text.h ?
*/

typedef struct lang_t lang;

struct lang_t {
    // Something here ...
};

void lang_create(lang *lang);
void lang_free(lang *lang);

// Return 0 on success
int lang_load(lang *lang, const char *filename);

// Maybe something like this ?
const char* lang_get(lang *lang, unsigned int textid);

#endif // _LANGUAGES_H