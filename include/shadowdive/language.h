#ifndef _SD_LANGUAGE_H
#define _SD_LANGUAGE_H

typedef struct sd_lang_string_t {
    unsigned int _offset;
    char description[33];
    char string[129];
} sd_lang_string;

typedef struct sd_language_t {
    unsigned int string_count;
    sd_lang_string *strings;
} sd_language;

#endif // _SD_LANGUAGE_H