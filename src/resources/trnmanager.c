#include "resources/trnmanager.h"
#include "formats/tournament.h"
#include "formats/error.h"
#include "resources/pathmanager.h"
#include "utils/list.h"
#include "utils/log.h"
#include "utils/scandir.h"
#include "utils/allocator.h"
#include <stdio.h>
#include <string.h>

sd_tournament_file** trnlist_init() {
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

    // TODO I assume this should be using a list
    sd_tournament_file **trns = omf_calloc(list_size(&dirlist), sizeof(sd_tournament_file));

    iterator it;
    list_iter_begin(&dirlist, &it);
    int i = 0;
    char *trn_file;
    while((trn_file = iter_next(&it)) != NULL) {
        sd_tournament_load(trns[i], trn_file);
    }

    list_free(&dirlist);
    return trns;

error_0:
    list_free(&dirlist);
    return NULL;
}

int trn_load(sd_tournament_file *trn, const char *trnname) {
    char tmp[1024];

    // Form the savegame filename
    const char *dirname = pm_get_local_path(RESOURCE_PATH);
    snprintf(tmp, 1024, "%s/%s.TRN", dirname, trnname);

    // Attempt to load
    sd_tournament_create(trn);
    int ret = sd_tournament_load(trn, tmp);
    if(ret != SD_SUCCESS) {
        PERROR("Unable to load tournament file '%s'.", tmp);
        return 1;
    }

    return 0;
}
