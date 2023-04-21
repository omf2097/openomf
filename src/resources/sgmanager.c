#include "resources/sgmanager.h"
#include "formats/chr.h"
#include "formats/error.h"
#include "resources/pathmanager.h"
#include "utils/list.h"
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
        return 1;
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

    // TODO: Handle looking up saves properly. Now just list_size - 2 (.. and .)
    DEBUG("Found %d savegames.", list_size(&dirlist) - 2);

    list_free(&dirlist);
    return 0;

error_0:
    list_free(&dirlist);
    return 1;
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

int sd_save(const sd_pilot *pilot, const char *pilotname) {
    // Report error for now
    return 1;
}
