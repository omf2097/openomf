#include "resources/sgmanager.h"
#include "formats/chr.h"
#include "formats/error.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/scandir.h"
#include <stdio.h>
#include <string.h>

int sg_init() {
    int ret;
    list dirlist;

    // Find path to savegame directory
    const char *dirname = pm_get_local_path(SAVE_PATH);
    if(dirname == NULL) {
        PERROR("Could not find save path! Something is wrong with path manager!");
        return 0;
    }

    // Seek all files
    list_create(&dirlist);
    ret = scan_directory(&dirlist, dirname);
    if(ret != 0) {
        INFO("Savegame directory does not exist. Attempting to create ...");
        if(pm_create_dir(dirname) != 0) {
            PERROR("Unable to create savegame directory!");
            goto error_0;
        } else {
            INFO("Savegame directory created at '%s'.", dirname);
        }

        // New attempt
        ret = scan_directory(&dirlist, dirname);
        if(ret != 0) {
            PERROR("Still could not read from savegame directory, giving up.");
            goto error_0;
        }
    }
    list_free(&dirlist);
    return 0;

error_0:
    list_free(&dirlist);
    return 1;
}

int sg_count() {
    if(sg_init()) {
        return 0;
    }
    const char *dirname = pm_get_local_path(SAVE_PATH);
    list dirlist;
    // Seek all files
    list_create(&dirlist);
    scan_directory(&dirlist, dirname);

    iterator it;
    char *filename = NULL;
    char *ext = NULL;

    list_iter_begin(&dirlist, &it);
    while((filename = (char *)list_iter_next(&it))) {
        if((ext = strrchr(filename, '.')) && strcmp(".CHR", ext) == 0) {
            continue;
        }
        DEBUG("ignoring file %s", filename);
        // not a CHR file, get lost
        list_delete(&dirlist, &it);
    }

    DEBUG("Found %d savegames.", list_size(&dirlist));
    int size = list_size(&dirlist);

    list_free(&dirlist);

    return size;
}

list *sg_load_all() {

    if(sg_init()) {
        return NULL;
    }

    const char *dirname = pm_get_local_path(SAVE_PATH);
    list dirlist;
    // Seek all files
    list_create(&dirlist);
    scan_directory(&dirlist, dirname);

    DEBUG("Found %d savegames.", list_size(&dirlist) - 2);

    list *chrlist = omf_calloc(1, sizeof(list));

    iterator it;
    list_iter_begin(&dirlist, &it);
    char *chrfile;
    char *ext;
    while((chrfile = (char *)list_iter_next(&it))) {
        if(strcmp(".", chrfile) == 0 || strcmp("..", chrfile) == 0) {
            continue;
        }
        if((ext = strrchr(chrfile, '.')) && strcmp(".CHR", ext) == 0) {
            sd_chr_file *chr = omf_calloc(1, sizeof(sd_chr_file));
            ext[0] = 0;
            DEBUG("%s", chrfile);
            sg_load(chr, chrfile);
            list_append(chrlist, chr, sizeof(sd_chr_file));
            omf_free(chr);
        }
    }

    list_free(&dirlist);
    return chrlist;
}

int sg_load(sd_chr_file *chr, const char *pilotname) {
    char tmp[1024];

    // Form the savegame filename
    const char *dirname = pm_get_local_path(SAVE_PATH);
    snprintf(tmp, 1024, "%s%s.CHR", dirname, pilotname);

    // Attempt to load
    sd_chr_create(chr);
    int ret = sd_chr_load(chr, tmp);
    if(ret != SD_SUCCESS) {
        PERROR("Unable to load savegame file '%s'.", tmp);
        return ret;
    }

    return SD_SUCCESS;
}
