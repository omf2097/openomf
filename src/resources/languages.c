#include "formats/language.h"
#include "formats/error.h"
#include "resources/languages.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/array.h"
#include "utils/log.h"

array language_strings;
sd_language *language;

int lang_init() {
    // Get filename
    const char *filename = pm_get_resource_path(DAT_ENGLISH);

    // Load up language file
    language = omf_calloc(1, sizeof(sd_language));
    if(sd_language_create(language) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language, filename)) {
        PERROR("Unable to load language file '%s'!", filename);
        goto error_1;
    }

    // Load language strings
    array_create(&language_strings);
    for(int i = 0; i < language->count; i++) {
        array_set(&language_strings, i, language->strings[i].data);
    }

    INFO("Loaded language file '%s'.", filename);
    return 0;

error_1:
    sd_language_free(language);
error_0:
    omf_free(language);
    return 1;
}

void lang_close() {
    array_free(&language_strings);
    sd_language_free(language);
    omf_free(language);
}

const char* lang_get(unsigned int id) {
    return (const char*)array_get(&language_strings, id);
}
