#include "resources/sgmanager.h"
#include "formats/chr.h"
#include "formats/error.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/io.h"
#include "utils/log.h"
#include "utils/scandir.h"
#include <errno.h>
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
    list_create(&dirlist);
    if(scan_directory_suffix(&dirlist, dirname, ".CHR") != 0) {
        log_warn("Failed to scan %s to find *.CHR", dirname);
        return NULL;
    }
    log_debug("Found %d savegames", list_size(&dirlist));

    list *chrlist = omf_calloc(1, sizeof(list));
    list_create(chrlist);
    iterator it;
    list_iter_begin(&dirlist, &it);
    char *chrfile;
    foreach(it, chrfile) {
        sd_chr_file *chr = omf_calloc(1, sizeof(sd_chr_file));
        if(sg_load(chr, chrfile) == SD_SUCCESS) {
            list_append(chrlist, chr, sizeof(sd_chr_file));
            log_debug("Loaded %s", chrfile);
        } else {
            log_warn("Failed to load save %s", chrfile);
        }
        omf_free(chr);
    }

    list_free(&dirlist);
    return chrlist;
}

int sg_load(sd_chr_file *chr, const char *pilotname) {
    str path;
    str_from_c(&path, pm_get_local_path(SAVE_PATH));
    str_append_c(&path, pilotname);
    size_t pilot_len = strlen(pilotname);
    if(pilot_len >= 4 && strcmp(&pilotname[pilot_len - 4], ".CHR") != 0)
        str_append_c(&path, ".CHR");

    sd_chr_create(chr);
    int ret = sd_chr_load(chr, str_c(&path));
    str_free(&path);
    return ret;
}

static void sg_fix_old_save(str *path, const char *pilotname) {
    str legacy_path;
    str_from(&legacy_path, path);
    sd_chr_append_legacy_filename(&legacy_path, pilotname);
    sd_chr_append_filename(path, pilotname);

    if(!str_equal(&legacy_path, path) && file_exists(str_c(&legacy_path))) {
        log_info("Renaming pilot %s save from %s to %s", pilotname, str_c(&legacy_path), str_c(path));
        if(rename(str_c(&legacy_path), str_c(path)) < 0) {
            log_warn("Failed to rename %s to %s: %s - Using old the old filename", str_c(&legacy_path), str_c(path),
                     strerror(errno));
            str_from(path, &legacy_path);
        }
    }

    str_free(&legacy_path);
}

int sg_save(sd_chr_file *chr) {
    str path;
    const char *dirname = pm_get_local_path(SAVE_PATH);
    str_from_c(&path, dirname);
    sg_fix_old_save(&path, chr->pilot.name);

    int ret = sd_chr_save(chr, str_c(&path));
    if(ret == SD_SUCCESS) {
        log_debug("Saved pilot %s in %s", chr->pilot.name, str_c(&path));
        str_cut_left(&path, strlen(dirname));
        omf_free(settings_get()->tournament.last_name);
        settings_get()->tournament.last_name = omf_strdup(str_c(&path));
        settings_save();
    }
    str_free(&path);
    return ret;
}

int sg_delete(const char *pilotname) {
    str path;
    str_from_c(&path, pm_get_local_path(SAVE_PATH));
    sd_chr_append_filename(&path, pilotname);

    int ret = remove(str_c(&path));
    if(ret != 0) {
        log_error("Failed to delete %s: %s", str_c(&path), strerror(errno));
    }
    str_free(&path);
    return ret;
}
