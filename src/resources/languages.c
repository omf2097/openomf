#include "resources/languages.h"

#include "formats/error.h"
#include "formats/language.h"
#include "game/utils/settings.h"
#include "resource_files.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "utils/path.h"
#include "utils/str.h"

static sd_language *language = NULL;
static sd_language *language2 = NULL;

// Fall back to the default language if the configured one does not resolve.
static void ensure_valid_language_setting(settings_language *lang_settings) {
    const path configured = get_resource_filename(lang_settings->language);
    if(!path_exists(&configured)) {
        log_error("Configured language file '%s' not found, falling back to 'ENGLISH.DAT'.", path_c(&configured));
        omf_free(lang_settings->language);
        lang_settings->language = omf_strdup("ENGLISH.DAT");
    }
}

bool lang_init(void) {
    language = NULL;
    language2 = NULL;

    settings_language *lang_settings = &settings_get()->language;
    ensure_valid_language_setting(lang_settings);
    const path language_file1 = get_resource_filename(lang_settings->language);

    // Load up language file
    language = omf_calloc(1, sizeof(sd_language));
    if(sd_language_create(language) != SD_SUCCESS) {
        goto error_0;
    }
    // Note: we don't load descriptions -- we don't use them.
    if(sd_language_load(language, &language_file1, false)) {
        log_error("Unable to load language file '%s'!", path_c(&language_file1));
        goto error_0;
    }

    // Trim off the last linebreak from the translation texts. We don't need it.
    for(unsigned int i = 0; i < vector_size(&language->strings); i++) {
        sd_lang_string *entry = vector_get(&language->strings, i);
        if(str_ends_with(&entry->data, "\n")) {
            str_cut(&entry->data, 1);
        }
    }

    // OMF GERMAN.DAT and old versions of ENGLISH.DAT have only 990 strings
    unsigned int const old_language_count = 990;

    if(vector_size(&language->strings) == old_language_count) {
        // OMF 2.1 added netplay, and with it 23 new localization strings. Insert empty
        // placeholders at their ids (ascending) so the original strings shift up to their
        // 1013-entry positions.
        const unsigned new_ids[] = {149, 150, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
                                    182, 183, 184, 185, 267, 269, 270, 271, 284, 295, 305};
        for(unsigned i = 0; i < N_ELEMENTS(new_ids); i++) {
            sd_lang_string dummy;
            str_from_c(&dummy.description, "dummy");
            str_create(&dummy.data);
            vector_insert_at(&language->strings, new_ids[i], &dummy);
        }
    }
    if(vector_size(&language->strings) != LANG_STR_COUNT) {
        log_error("Unable to load language file '%s', unsupported or corrupt file!", path_c(&language_file1));
        goto error_0;
    }

    log_info("Loaded language file '%s'.", path_c(&language_file1));

    // Load up language2 file (OpenOMF)
    path language_file2 = get_resource_filename(lang_settings->language);
    str ext;
    path_ext(&language_file2, &ext);
    str_append_char(&ext, '2');
    path_set_ext(&language_file2, str_c(&ext));
    str_free(&ext);

    language2 = omf_calloc(1, sizeof(sd_language));
    if(sd_language_create(language2) != SD_SUCCESS) {
        goto error_0;
    }
    // Note: we don't load descriptions -- we don't use them.
    if(sd_language_load(language2, &language_file2, false)) {
        log_error("Unable to load OpenOMF language file '%s'!", path_c(&language_file2));
        goto error_0;
    }
    if(vector_size(&language2->strings) != LANG2_STR_COUNT) {
        log_error("Unable to load OpenOMF language file '%s', unsupported or corrupt file!", path_c(&language_file2));
        goto error_0;
    }

    log_info("Loaded OpenOMF language file '%s'.", path_c(&language_file2));

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
    const sd_lang_string *entry = vector_get(&language->strings, id);
    if(entry == NULL || str_size(&entry->data) == 0) {
        log_error("unsupported lang id %u!", id);
        return "!INVALID!";
    }
    return str_c(&entry->data);
}

const char *lang_get2(unsigned int id) {
    const sd_lang_string *entry = vector_get(&language2->strings, id);
    if(entry == NULL || str_size(&entry->data) == 0) {
        log_error("unsupported lang2 id %u!", id);
        return "!INVALID2!";
    }
    return str_c(&entry->data);
}
