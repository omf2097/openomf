#include "resources/trnmanager.h"
#include "formats/error.h"
#include "formats/tournament.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/scandir.h"
#include "utils/vector.h"
#include <stdio.h>
#include <string.h>

static int trn_sort_compare_fn(const void *a, const void *b) {
    sd_tournament_file const *trn_a = a;
    sd_tournament_file const *trn_b = b;
    return (trn_a->registration_fee > trn_b->registration_fee) - (trn_a->registration_fee < trn_b->registration_fee);
}

void trnlist_init(vector *trnlist) {
    trnlist_free(trnlist);

    int ret;
    list dirlist;

    // Find path to savegame directory
    const char *dirname = pm_get_local_path(RESOURCE_PATH);
    if(dirname == NULL) {
        log_error("Could not find resources path! Something is wrong with tournament manager!");
        return;
    }

    // Seek all files
    list_create(&dirlist);
    ret = scan_directory_suffix(&dirlist, dirname, ".TRN");
    if(ret != 0) {
        goto error_0;
    }

    log_debug("Found %d tournaments.", list_size(&dirlist));

    vector_create(trnlist, sizeof(sd_tournament_file));

    iterator it;
    list_iter_begin(&dirlist, &it);
    char *trn_file;
    char tmp[1024];
    foreach(it, trn_file) {
        sd_tournament_file trn;
        sd_tournament_create(&trn);
        snprintf(tmp, 1024, "%s%s", dirname, trn_file);
        if(SD_SUCCESS == sd_tournament_load(&trn, tmp)) {
            vector_append(trnlist, &trn);
        } else {
            log_error("Could not load tournament %s", trn_file);
        }
    }
    list_iter_end(&dirlist, &it);

    // sort the tournaments by ascending registration fee
    vector_sort(trnlist, trn_sort_compare_fn);

    log_debug("Loaded %d tournaments", vector_size(trnlist));

error_0:
    list_free(&dirlist);
}

void trnlist_free(vector *trnlist) {
    iterator it;
    vector_iter_begin(trnlist, &it);
    sd_tournament_file *trn;
    foreach(it, trn) {
        sd_tournament_free(trn);
    }

    vector_free(trnlist);
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
        log_error("Unable to load tournament file '%s'.", tmp);
        return 1;
    }

    return 0;
}
