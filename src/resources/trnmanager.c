#include "resources/trnmanager.h"
#include "formats/error.h"
#include "formats/tournament.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/list.h"
#include "utils/log.h"
#include "utils/scandir.h"
#include <stdio.h>
#include <string.h>

static void trnlist_node_free_callback(void *data) {
    sd_tournament_file *trn = data;
    sd_tournament_free(trn);
}

list *trnlist_init(void) {
    int ret;
    list dirlist;

    // Find path to savegame directory
    const char *dirname = pm_get_local_path(RESOURCE_PATH);
    if(dirname == NULL) {
        PERROR("Could not find resources path! Something is wrong with tournament manager!");
        return NULL;
    }

    // Seek all files
    list_create(&dirlist);
    ret = scan_directory_suffix(&dirlist, dirname, ".TRN");
    if(ret != 0) {
        goto error_0;
    }

    DEBUG("Found %d tournaments.", list_size(&dirlist));

    list *trnlist = omf_calloc(1, sizeof(list));
    list_create(trnlist);
    list_set_node_free_cb(trnlist, trnlist_node_free_callback);

    iterator it;
    list_iter_begin(&dirlist, &it);
    char *trn_file;
    char tmp[1024];
    while((trn_file = iter_next(&it)) != NULL) {
        sd_tournament_file trn;
        sd_tournament_create(&trn);
        snprintf(tmp, 1024, "%s%s", dirname, trn_file);
        if(SD_SUCCESS == sd_tournament_load(&trn, tmp)) {
            list_append(trnlist, &trn, sizeof(sd_tournament_file));
        } else {
            PERROR("Could not load tournament %s", trn_file);
        }
    }
    list_iter_end(&dirlist, &it);

    DEBUG("Loaded %d tournaments", list_size(trnlist));

    list_free(&dirlist);
    return trnlist;

error_0:
    list_free(&dirlist);
    return NULL;
}

int trn_load(sd_tournament_file *trn, const char *trnname) {
    char tmp[1024];

    // Form the savegame filename
    const char *dirname = pm_get_local_path(RESOURCE_PATH);
    snprintf(tmp, 1024, "%s%s", dirname, trnname);

    // Attempt to load
    sd_tournament_create(trn);
    int ret = sd_tournament_load(trn, tmp);
    if(ret != SD_SUCCESS) {
        PERROR("Unable to load tournament file '%s'.", tmp);
        return 1;
    }

    return 0;
}
