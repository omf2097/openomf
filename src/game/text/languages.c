#include <shadowdive/shadowdive.h>
#include "utils/array.h"
#include "utils/log.h"
#include "game/text/languages.h"

array language_strings;
sd_language *language;

int lang_init() {
    language = sd_language_create();
    if(sd_language_load(language, "resources/ENGLISH.DAT")) {
        PERROR("Language file could not be loaded!\n");
        return 1;
    }
    array_create(&language_strings);
    for(int i = 0; i < language->count; i++) {
        array_set(&language_strings, i, language->strings[i].data);
    }
    return 0;
}

void lang_close() {
    array_free(&language_strings);
    sd_language_delete(language);
}

const char* lang_get(unsigned int id) {
    return (const char*)array_get(&language_strings, id);
}


