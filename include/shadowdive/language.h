#ifndef _SD_LANGUAGE_H
#define _SD_LANGUAGE_H

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    char description[32];
    char *data;
} sd_lang_string;

typedef struct {
    unsigned int count;
    sd_lang_string *strings;
} sd_language;

int sd_language_create(sd_language *language);
void sd_language_free(sd_language *language);
int sd_language_load(sd_language *language, const char *filename);
int sd_language_save(sd_language *language, const char *filename);

sd_lang_string* sd_language_get(const sd_language *language, int num);

#ifdef __cplusplus
}
#endif

#endif // _SD_LANGUAGE_H
