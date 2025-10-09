#include "resources/trnmanager.h"

#include "formats/error.h"
#include "formats/tournament.h"
#include "resource_files.h"
#include "resources/modmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/vector.h"
#include <stdio.h>

static int trn_sort_compare_fn(const void *a, const void *b) {
    sd_tournament_file const *trn_a = a;
    sd_tournament_file const *trn_b = b;
    return (trn_a->registration_fee > trn_b->registration_fee) - (trn_a->registration_fee < trn_b->registration_fee);
}

void trnlist_init(vector *trnlist) {
    trnlist_free(trnlist);

    // Seek all tournament files
    list dir_list;
    list_create(&dir_list);
    if(!scan_resource_path(&dir_list, "*.TRN")) {
        log_error("Could not scan resources path!");
        goto error_0;
    }
    log_debug("Found %d tournaments.", list_size(&dir_list));

    vector_create(trnlist, sizeof(sd_tournament_file));
    iterator it;
    list_iter_begin(&dir_list, &it);
    path *trn_file;
    foreach(it, trn_file) {
        sd_tournament_file trn;
        sd_tournament_create(&trn);
        if(SD_SUCCESS == sd_tournament_load(&trn, trn_file)) {
            // apply any mods
            str fn;
            path_filename(trn_file, &fn);
            modmanager_get_tournament_mod(str_c(&fn), &trn);
            str_free(&fn);
            vector_append(trnlist, &trn);
        } else {
            log_error("Could not load tournament %s", path_c(trn_file));
        }
    }

    // TODO query the modmanager for "pure" mod tournaments

    // sort the tournaments by ascending registration fee
    vector_sort(trnlist, trn_sort_compare_fn);
    log_debug("Loaded %d tournaments", vector_size(trnlist));

error_0:
    list_free(&dir_list);
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

int trn_load(sd_tournament_file *trn, const char *trn_name) {
    const path trn_file = get_resource_filename(trn_name);
    sd_tournament_create(trn);
    int ret = sd_tournament_load(trn, &trn_file);
    if(ret != SD_SUCCESS) {
        log_error("Unable to load tournament file '%s'.", path_c(&trn_file));
        return 1;
    }

    // apply any mods
    modmanager_get_tournament_mod(trn_name, trn);
    return 0;
}
