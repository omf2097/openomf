#include <stdio.h>

#include "game/scenes/mainmenu/menu_language.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "resources/languages.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/list.h"
#include "utils/log.h"
#include "utils/scandir.h"

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
    scan_directory(&dirlist, dirname);
    local->language_filenames = omf_malloc(list_size(&dirlist) * sizeof(char *));
    local->language_names = omf_malloc(list_size(&dirlist) * sizeof(char *));
    local->language_count = 0;

    iterator it;
    list_iter_begin(&dirlist, &it);
    char const *filename;
    while((filename = (char *)list_iter_next(&it))) {
        char *ext = NULL;

        if(strcmp("ENGLISH.DAT", filename) != 0 && strcmp("GERMAN.DAT", filename) != 0 &&
           ((ext = strrchr(filename, '.')) == NULL || strcmp(".LNG", ext) != 0)) {
            continue;
        }

        if(strcmp(setting->language.language, filename) == 0) {
            local->selected_language = local->language_count;
        }

        int id = local->language_count++;

        // strip .DAT or .LNG
        size_t filename_len = strlen(filename);
        size_t name_len = filename_len - 4;
        local->language_names[id] = omf_malloc(name_len + 1);
        memcpy(local->language_names[id], filename, name_len);
        local->language_names[id][name_len] = '\0';

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
