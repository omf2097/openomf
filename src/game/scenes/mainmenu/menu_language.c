#include <stdio.h>

#include "game/scenes/mainmenu/menu_language.h"

#include "formats/error.h"
#include "formats/language.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "resources/languages.h"
#include "resources/resource_files.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/list.h"
#include "utils/log.h"
#include "utils/path.h"
#include "utils/str.h"

typedef struct {
    char **language_filenames;
    char **language_names;
    int language_count;
    int selected_language;
} language_menu_data;

void menu_language_done(component *c, void *u) {
    language_menu_data *local = menu_get_userdata(c->parent);
    settings_language *l = &settings_get()->language;

    // Set menu as finished
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;

    if(strcmp(l->language, local->language_filenames[local->selected_language]) != 0) {
        omf_free(l->language);
        l->language = local->language_filenames[local->selected_language];
        local->language_filenames[local->selected_language] = NULL;

        // reload language
        lang_close();
        lang_init();
    }
}

void menu_language_free(component *c) {
    language_menu_data *local = menu_get_userdata(c);
    for(int l = 0; l < local->language_count; l++) {
        omf_free(local->language_filenames[l]);
        omf_free(local->language_names[l]);
    }
    omf_free(local->language_filenames);
    omf_free(local->language_names);
    omf_free(local);
    menu_set_userdata(c, local);
}

void menu_language_submenu_done(component *c, component *submenu) {
    menu *m = sizer_get_obj(c);
    m->finished = 1;
}

component *menu_language_create(scene *s) {
    // Menu userdata
    language_menu_data *local = omf_calloc(1, sizeof(language_menu_data));

    // Load settings etc.
    const settings *setting = settings_get();
    const path english_dat = get_resource_filename("ENGLISH.DAT");
    const path german_dat = get_resource_filename("GERMAN.DAT");

    list dir_list;
    list_create(&dir_list);
    list_append(&dir_list, &english_dat, sizeof(path));
    list_append(&dir_list, &german_dat, sizeof(path));
    scan_resource_path(&dir_list, "*.LNG");

    local->language_filenames = omf_malloc(list_size(&dir_list) * sizeof(char *));
    local->language_names = omf_malloc(list_size(&dir_list) * sizeof(char *));
    local->language_count = 0;

    str name, ext;
    iterator it;
    list_iter_begin(&dir_list, &it);
    path *language_file2;
    foreach(it, language_file2) {
        // The base filename (e.g. "ENGLISH.DAT") is the language identifier stored in settings.
        // Capture it before we mutate the path to point at the .DAT2/.LNG2 file below.
        path_filename(language_file2, &name);

        // Switch to the OpenOMF .DAT2 / .LNG2 supplementary file to read the localized language name.
        path_ext(language_file2, &ext);
        str_append_char(&ext, '2');
        path_set_ext(language_file2, str_c(&ext));
        str_free(&ext);

        sd_language lang2;
        if(sd_language_create(&lang2) != SD_SUCCESS) {
            str_free(&name);
            continue;
        }
        if(sd_language_load(&lang2, language_file2, false)) {
            log_info("Warning: Unable to load OpenOMF language file '%s'!", path_c(language_file2));
            sd_language_free(&lang2);
            str_free(&name);
            continue;
        }
        if(vector_size(&lang2.strings) != LANG2_STR_COUNT) {
            log_info("Warning: Invalid OpenOMF language file '%s', got %u entries!", path_c(language_file2),
                     vector_size(&lang2.strings));
            sd_language_free(&lang2);
            str_free(&name);
            continue;
        }
        const sd_lang_string *lang_entry = vector_get(&lang2.strings, LANG2_STR_LANGUAGE);
        char *language_name = omf_strdup(str_c(&lang_entry->data));
        sd_language_free(&lang2);

        if(str_match(&name, setting->language.language)) {
            local->selected_language = local->language_count;
        }

        const int id = local->language_count++;
        local->language_names[id] = language_name;
        local->language_filenames[id] = omf_strdup(str_c(&name));
        str_free(&name);
    }
    list_free(&dir_list);

    // Create menu and its header
    component *menu = menu_create();
    menu_attach(menu, label_create_title("LANGUAGE"));

    menu_attach(menu, textselector_create_bind_opts("", "Choose a Language.", NULL, NULL, &local->selected_language,
                                                    (char const **)local->language_names, local->language_count));

    menu_attach(menu, filler_create());

    // Done button
    menu_attach(menu, button_create("DONE", "Return to the main menu.", false, false, menu_language_done, s));

    // Userdata & free function for it
    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_language_free);
    menu_set_submenu_done_cb(menu, menu_language_submenu_done);
    return menu;
}
