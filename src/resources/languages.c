#include "resources/languages.h"

#include "formats/error.h"
#include "formats/language.h"
#include "game/utils/settings.h"
#include "resource_files.h"
#include "resource_paths.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/str.h"
#include <string.h>

static sd_language *language = NULL;
static sd_language *language2 = NULL;

bool lang_init(void) {
    language = NULL;
    language2 = NULL;

    path language_file1, language_file2;
    language_file1 = language_file2 = get_resource_filename(settings_get()->language.language);

    // Load up language file
    language = omf_calloc(1, sizeof(sd_language));
    if(sd_language_create(language) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language, &language_file1)) {
        log_error("Unable to load language file '%s'!", path_c(&language_file1));
        goto error_0;
    }

    // Trim off the last linebreak from the translation texts. We don't need it.
    for(unsigned int i = 0; i < language->count; i++) {
        size_t len = strlen(language->strings[i].data);
        if(len > 0 && language->strings[i].data[len - 1] == '\n') {
            language->strings[i].data[len - 1] = '\0';
        }
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
        log_error("Unable to load language file '%s', unsupported or corrupt file!", path_c(&language_file1));
        goto error_0;
    }

    log_info("Loaded language file '%s'.", path_c(&language_file1));

    // Load up language2 file (OpenOMF)
    str ext;
    path_ext(&language_file2, &ext);
    str_append_char(&ext, '2');
    path_set_ext(&language_file2, str_c(&ext));
    str_free(&ext);

    language2 = omf_calloc(1, sizeof(sd_language));
    if(sd_language_create(language2) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language2, &language_file2)) {
        log_error("Unable to load OpenOMF language file '%s'!", path_c(&language_file2));
        goto error_0;
    }
    if(language2->count != LANG2_STR_COUNT) {
        log_error("Unable to load OpenOMF language file '%s', unsupported or corrupt file!", path_c(&language_file2));
        goto error_0;
    }

    log_info("Loaded OpenOMF language file '%s'.", path_c(&language_file2));

    // XXX we're wasting 32KB of memory on language->strings[...].description

    return true;

error_0:
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
