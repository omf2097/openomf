#include <stdio.h>

#include "game/scenes/mainmenu/menu_language.h"

#include "formats/error.h"
#include "formats/language.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "resources/languages.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/list.h"
#include "utils/log.h"
#include "utils/scandir.h"
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
    settings *setting = settings_get();

    // Find path to languages
    const char *dirname = pm_get_local_path(RESOURCE_PATH);
    if(dirname == NULL) {
        PERROR("Could not find resources path for menu_language!");
        return NULL;
    }

    list dirlist;
    // Seek all files
    list_create(&dirlist);
    list_append(&dirlist, "ENGLISH.DAT", strlen("ENGLISH.DAT") + 1);
    list_append(&dirlist, "GERMAN.DAT", strlen("GERMAN.DAT") + 1);
    scan_directory_suffix(&dirlist, dirname, ".LNG");
    local->language_filenames = omf_malloc(list_size(&dirlist) * sizeof(char *));
    local->language_names = omf_malloc(list_size(&dirlist) * sizeof(char *));
    local->language_count = 0;

    iterator it;
    list_iter_begin(&dirlist, &it);
    char const *filename;
    str filename2;
    str_create(&filename2);
    while((filename = (char *)list_iter_next(&it))) {
        // Get localized language name from OpenOMF .DAT2 or .LNG2 file
        str_format(&filename2, "%s%s2", dirname, filename);
        sd_language lang2;
        if(sd_language_create(&lang2) != SD_SUCCESS) {
            continue;
        }
        if(sd_language_load(&lang2, str_c(&filename2))) {
            INFO("Warning: Unable to load OpenOMF language file '%s'!", str_c(&filename2));
            sd_language_free(&lang2);
            continue;
        }
        if(lang2.count != LANG2_STR_COUNT) {
            INFO("Warning: Invalid OpenOMF language file '%s', got %d entries!", str_c(&filename2), lang2.count);
            sd_language_free(&lang2);
            continue;
        }
        char *language_name = lang2.strings[LANG2_STR_LANGUAGE].data;
        lang2.strings[LANG2_STR_LANGUAGE].data = NULL;
        sd_language_free(&lang2);

        if(strcmp(setting->language.language, filename) == 0) {
            local->selected_language = local->language_count;
        }

        int id = local->language_count++;
        local->language_names[id] = language_name;

        // move filename into language_filenames
        list_node *now = it.vnow;
        local->language_filenames[id] = now->data;
        now->data = NULL;
    }
    list_free(&dirlist);

    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = TEXT_MEDIUM_GREEN;

    // Create menu and its header
    component *menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "LANGUAGE"));

    menu_attach(menu,
                textselector_create_bind_opts(&tconf, "", "Choose a Language.", NULL, NULL, &local->selected_language,
                                              (char const **)local->language_names, local->language_count));

    menu_attach(menu, filler_create());

    // Done button
    menu_attach(menu,
                textbutton_create(&tconf, "DONE", "Return to the main menu.", COM_ENABLED, menu_language_done, s));

    // Userdata & free function for it
    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_language_free);
    menu_set_submenu_done_cb(menu, menu_language_submenu_done);
    return menu;
}
