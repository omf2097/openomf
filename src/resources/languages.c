#include "resources/languages.h"
#include "formats/error.h"
#include "formats/language.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/str.h"
#include <string.h>

static sd_language *language;
static sd_language *language2;

bool lang_init(void) {
    language = NULL;
    language2 = NULL;

    str filename_str;
    const char *dirname = pm_get_local_path(RESOURCE_PATH);
    const char *lang = settings_get()->language.language;
    str_from_format(&filename_str, "%s%s", dirname, lang);
    char const *filename = str_c(&filename_str);

    // Load up language file
    language = omf_calloc(1, sizeof(sd_language));
    if(sd_language_create(language) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language, filename)) {
        log_error("Unable to load language file '%s'!", filename);
        goto error_0;
    }

    // OMF GERMAN.DAT and old versions of ENGLISH.DAT have only 990 strings
    unsigned int const old_language_count = 990;

    if(language->count == old_language_count) {
        // OMF 2.1 added netplay, and with it 23 new localization strings
        unsigned new_ids[] = {149, 150, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
                              182, 183, 184, 185, 267, 269, 270, 271, 284, 295, 305};
        unsigned *new_ids_end = new_ids + N_ELEMENTS(new_ids);

        // insert dummy entries
        sd_lang_string *expanded_strings = omf_malloc(LANG_STR_COUNT * sizeof(sd_lang_string));
        unsigned next = 0;
        unsigned next_from = 0;
        for(unsigned *id = new_ids; id < new_ids_end; id++) {
            unsigned copy_count = *id - next;
            memcpy(expanded_strings + next, language->strings + next_from, copy_count * sizeof(sd_lang_string));
            next += copy_count;
            next_from += copy_count;

            expanded_strings[next].data = NULL;
            memcpy(expanded_strings[next].description, "dummy", 6);
            next++;
            language->count++;
        }
        memcpy(expanded_strings + next, language->strings + next_from,
               (LANG_STR_COUNT - next) * sizeof(sd_lang_string));
        omf_free(language->strings);
        language->strings = expanded_strings;
    }
    if(language->count != LANG_STR_COUNT) {
        log_error("Unable to load language file '%s', unsupported or corrupt file!", filename);
        goto error_0;
    }

    log_info("Loaded language file '%s'.", filename);

    // Load up language2 file (OpenOMF)
    str_append_c(&filename_str, "2");
    filename = str_c(&filename_str);

    language2 = omf_calloc(1, sizeof(sd_language));
    if(sd_language_create(language2) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language2, filename)) {
        log_error("Unable to load OpenOMF language file '%s'!", filename);
        goto error_0;
    }
    if(language2->count != LANG2_STR_COUNT) {
        log_error("Unable to load OpenOMF language file '%s', unsupported or corrupt file!", filename);
        goto error_0;
    }

    log_info("Loaded OpenOMF language file '%s'.", filename);

    str_free(&filename_str);

    // XXX we're wasting 32KB of memory on language->strings[...].description

    return true;

error_0:
    str_free(&filename_str);
    lang_close();
    return false;
}

void lang_close(void) {
    sd_language_free(language);
    omf_free(language);
    sd_language_free(language2);
    omf_free(language2);
}

const char *lang_get(unsigned int id) {
    if(id > language->count || !language->strings[id].data) {
        log_error("unsupported lang id %u!", id);
        return "!INVALID!";
    }
    return language->strings[id].data;
}

const char *lang_get2(unsigned int id) {
    if(id > language2->count || !language2->strings[id].data) {
        log_error("unsupported lang2 id %u!", id);
        return "!INVALID2!";
    }
    return language2->strings[id].data;
}
