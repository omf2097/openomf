#include <shadowdive/shadowdive.h>
#include "utils/array.h"
#include "utils/log.h"
#include "game/text/languages.h"
#include "resources/ids.h"

array language_strings;
sd_language *language;

int lang_init() {
    // Get filename
    char filename[64];
    get_filename_by_id(DAT_ENGLISH, filename);

    // Load up language file
    language = sd_language_create();
    if(sd_language_load(language, filename)) {
        PERROR("Unable to load language file '%s'!", filename);
        return 1;
    }
    array_create(&language_strings);
    for(int i = 0; i < language->count; i++) {
        array_set(&language_strings, i, language->strings[i].data);
    }

    DEBUG("Loaded language file '%s'.", filename);
    return 0;
}

void lang_close() {
    array_free(&language_strings);
    sd_language_delete(language);
}

const char* lang_get(unsigned int id) {
    return (const char*)array_get(&language_strings, id);
}


