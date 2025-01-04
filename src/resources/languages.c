#include "resources/languages.h"
#include "formats/error.h"
#include "formats/language.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/str.h"
#include <assert.h>
#include <string.h>

static sd_language *language;

bool lang_init(void) {
    language = NULL;

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
    language = omf_calloc(1, sizeof(sd_language));
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

    INFO("Loaded language file '%s'.", filename);

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
}

const char *lang_get(int id) {
    assert(id >= 0);
    assert(id < Lang_Count);
    return language->strings[id].data;
}

const char *lang_get_offset_impl(int id, int last, int offset) {
    assert(id >= 0);
    assert(last >= id);
    assert(last < Lang_Count);
    assert(id + offset <= last);
    return lang_get(id + offset);
}
