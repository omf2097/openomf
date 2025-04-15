#include "resources/sgmanager.h"
#include "formats/chr.h"
#include "formats/error.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "utils/scandir.h"
#include <stdio.h>
#include <string.h>

int sg_init(void) {
    int ret;
    list dirlist;

    // Find path to savegame directory
    const char *dirname = pm_get_local_path(SAVE_PATH);
    if(dirname == NULL) {
        log_error("Could not find save path! Something is wrong with path manager!");
        return 0;
    }

    // Seek all files
    list_create(&dirlist);
    ret = scan_directory(&dirlist, dirname);
    if(ret != 0) {
        log_info("Savegame directory does not exist. Attempting to create ...");
        if(pm_create_dir(dirname) != 0) {
            log_error("Unable to create savegame directory!");
            goto error_0;
        } else {
            log_info("Savegame directory created at '%s'.", dirname);
        }

        // New attempt
        ret = scan_directory(&dirlist, dirname);
        if(ret != 0) {
            log_error("Still could not read from savegame directory, giving up.");
            goto error_0;
        }
    }
    list_free(&dirlist);
    return 0;

error_0:
    list_free(&dirlist);
    return 1;
}

int sg_count(void) {
    if(sg_init()) {
        return 0;
    }
    const char *dirname = pm_get_local_path(SAVE_PATH);
    list dirlist;
    list_create(&dirlist);
    int ret;
    if(scan_directory_suffix(&dirlist, dirname, ".CHR") != 0) {
        ret = 0;
        log_warn("Failed to scan %s to find *.CHR", dirname);
    } else {
        ret = list_size(&dirlist);
        log_debug("Found %d savegames.", ret);
    }
    list_free(&dirlist);
    return ret;
}

list *sg_load_all(void) {

    if(sg_init()) {
        return NULL;
    }

    const char *dirname = pm_get_local_path(SAVE_PATH);
    list dirlist;
    // Seek all files
    list_create(&dirlist);
    scan_directory(&dirlist, dirname);

    log_debug("Found %d savegames.", list_size(&dirlist) - 2);

    list *chrlist = omf_calloc(1, sizeof(list));

    iterator it;
    list_iter_begin(&dirlist, &it);
    char *chrfile;
    char *ext;
    foreach(it, chrfile) {
        if(strcmp(".", chrfile) == 0 || strcmp("..", chrfile) == 0) {
            continue;
        }
        if((ext = strrchr(chrfile, '.')) && strcmp(".CHR", ext) == 0) {
            sd_chr_file *chr = omf_calloc(1, sizeof(sd_chr_file));
            ext[0] = 0;
            log_debug("%s", chrfile);
            if(sg_load(chr, chrfile) == SD_SUCCESS) {
                list_append(chrlist, chr, sizeof(sd_chr_file));
            }
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
        log_error("Unable to load savegame file '%s'.", tmp);
        return ret;
    }

    // Load colors from other files
    sd_pilot_set_player_color(&chr->pilot, PRIMARY, chr->pilot.color_1);
    sd_pilot_set_player_color(&chr->pilot, SECONDARY, chr->pilot.color_2);
    sd_pilot_set_player_color(&chr->pilot, TERTIARY, chr->pilot.color_3);

    return SD_SUCCESS;
}

int sg_save(sd_chr_file *chr) {
    char filename[1024];
    const char *dirname = pm_get_local_path(SAVE_PATH);
    snprintf(filename, sizeof(filename), "%s%s.CHR", dirname, chr->pilot.name);
    int ret = sd_chr_save(chr, filename);
    if(ret == SD_SUCCESS) {
        log_debug("Saved pilot %s in %s", chr->pilot.name, filename);
        omf_free(settings_get()->tournament.last_name);
        settings_get()->tournament.last_name = omf_strdup(chr->pilot.name);
        settings_save();
    }
    return ret;
}

int sg_delete(const char *pilotname) {
    char pathname[1024];

    const char *dirname = pm_get_local_path(SAVE_PATH);
    snprintf(pathname, sizeof(pathname), "%s%s.CHR", dirname, pilotname);
    int ret = remove(pathname);
    if(ret != 0) {
        log_error("Failed to delete %s: %m", pathname);
        return SD_FILE_UNLINK_ERROR;
    }
    return SD_SUCCESS;
}
