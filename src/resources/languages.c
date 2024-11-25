#include "resources/languages.h"
#include "formats/error.h"
#include "formats/language.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/str.h"
#include <assert.h>
#include <string.h>

static char **language_compact;

static char **lang_compact(sd_language *lang) {
    // calculate in-memory size of language's data
    unsigned int data_byte_count = 0;
    for(unsigned int i = 0; i < lang->count; i++) {
        char const *data = lang->strings[i].data;
        assert(data);
        data_byte_count += strlen(data) + 1;
    }

    size_t alloc_size = lang->count * sizeof(char *) + data_byte_count;
    char **allocation = omf_malloc(alloc_size);
    char *strings = (char *)(allocation + lang->count);

    DEBUG("Packing %u language strings into %zu bytes.", lang->count, alloc_size);

    // fill the allocation
    char **ptr_iter = allocation;
    char *strings_iter = strings;
    for(unsigned int i = 0; i < lang->count; i++) {
        *(ptr_iter++) = strings_iter;

        char const *data = lang->strings[i].data;
        size_t data_size = strlen(data) + 1;
        memcpy(strings_iter, data, data_size);
        strings_iter += data_size;
    }

    assert(ptr_iter == allocation + lang->count);
    assert(strings_iter == strings + data_byte_count);

    return allocation;
}

bool lang_init(void) {
    language_compact = NULL;
    sd_language *language = NULL;

    char *lang = settings_get()->language.language;

    {
        char *old_extension = strstr(lang, ".DAT");
        if(old_extension)
            memcpy(old_extension, ".LNG", 4);
    }

    str filename_str;
    const char *dirname = pm_get_local_path(RESOURCE_PATH);
    str_from_format(&filename_str, "%s%s", dirname, lang);
    char const *filename = str_c(&filename_str);

    // Load up language file
    sd_language language_real;
    language = &language_real;
    if(sd_language_create(language) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language, filename)) {
        PERROR("Unable to load language file '%s'!", filename);
        goto error_0;
    }

    if(language->count != Lang_Count) {
        PERROR("Unable to load language file '%s', unsupported or corrupt file!", filename);
        goto error_0;
    }

    language_compact = lang_compact(language);

    INFO("Loaded language file '%s'.", filename);

    str_free(&filename_str);
    sd_language_free(language);

    return true;

error_0:
    str_free(&filename_str);
    sd_language_free(language);
    lang_close();
    return false;
}

void lang_close(void) {
    omf_free(language_compact);
}

const char *lang_get(unsigned int id) {
    assert(id < Lang_Count);
    return language_compact[id];
}

const char *lang_get_offset_impl(unsigned int id, unsigned int id_last, unsigned int offset) {
    assert((id + offset) <= id_last);
    return lang_get(id + offset);
}
