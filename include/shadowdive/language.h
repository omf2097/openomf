#ifndef _SD_LANGUAGE_H
#define _SD_LANGUAGE_H

typedef struct sd_lang_string_t {
    char description[33];
    char *data;
} sd_lang_string;

typedef struct sd_language_t {
    unsigned int count;
    sd_lang_string *strings;
} sd_language;

sd_language* sd_language_create();
void sd_language_delete(sd_language *language);
int sd_language_load(sd_language *language, const char *filename);
int sd_language_save(sd_language *language, const char *filename);

#endif // _SD_LANGUAGE_H
