#include "shadowdive/language.h"
#include <stdlib.h>

sd_language* sd_language_create() {
    sd_language *language = (sd_language*)malloc(sizeof(sd_language));
    language->string_count = 0;
    language->strings = 0;
    return language;
}

void sd_language_delete(sd_language *language) {
    if(language) {
        free(language);
    }
}

int sd_language_load(sd_language *language, const char *filename) {
    return 0;
}

int sd_language_save(sd_language *language, const char *filename) {
    return 0;
}
