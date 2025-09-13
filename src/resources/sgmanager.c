#include "resources/sgmanager.h"

#include "formats/chr.h"
#include "formats/error.h"
#include "game/utils/settings.h"
#include "resource_files.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "utils/scandir.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

int sg_count(void) {
    list dir_list;
    list_create(&dir_list);
    if(!scan_save_directory(&dir_list, "*.CHR")) {
        log_warn("Failed to scan savegame directory!");
    } else {
        log_info("Found %d saved games.", list_size(&dir_list));
    }
    const int size = list_size(&dir_list);
    list_free(&dir_list);
    return size;
}

list *sg_load_all(void) {
    const path savegame_path = get_save_directory();
    const char *dirname = path_c(&savegame_path);
    list dir_list;
    list_create(&dir_list);
    if(!scan_save_directory(&dir_list, "*.CHR")) {
        log_error("Failed to scan savegame directory!", dirname);
        return NULL;
    }
    log_debug("Found %d saved games", list_size(&dir_list));

    list *chr_list = omf_calloc(1, sizeof(list));
    list_create(chr_list);
    iterator it;
    list_iter_begin(&dir_list, &it);
    path *filename;
    foreach(it, filename) {
        sd_chr_file *chr = omf_calloc(1, sizeof(sd_chr_file));
        if(sg_load(chr, filename) == SD_SUCCESS) {
            list_append(chr_list, chr, sizeof(sd_chr_file));
            log_debug("Loaded %s", path_c(filename));
        } else {
            log_warn("Failed to load save %s", path_c(filename));
        }
        omf_free(chr);
    }

    list_free(&dir_list);
    return chr_list;
}

int sg_load(sd_chr_file *chr, const path *file_name) {
    sd_chr_create(chr);
    const int ret = sd_chr_load(chr, path_c(file_name));
    if(ret != SD_SUCCESS) {
        log_error("Loading savegame %s failed: %s", path_c(file_name), sd_get_error(ret));
    } else {
        log_info("Loaded savegamne from %s", path_c(file_name));
    }
    return ret;
}

int sg_load_pilot(sd_chr_file *chr, const char *pilot_name) {
    path save = get_save_directory();
    path_append(&save, pilot_name);
    path_set_ext(&save, ".CHR");
    path_dossify_filename(&save);
    return sg_load(chr, &save);
}

int sg_save(sd_chr_file *chr) {
    path save = get_save_directory();
    path_append(&save, chr->pilot.name);
    path_set_ext(&save, ".CHR");
    path_dossify_filename(&save);

    const int ret = sd_chr_save(chr, path_c(&save));
    if(ret != SD_SUCCESS) {
        log_error("Saving pilot %s to %s failed: %s", chr->pilot.name, path_c(&save), strerror(errno));
    } else {
        log_info("Saved pilot %s to %s", chr->pilot.name, path_c(&save));
        str stem;
        path_stem(&save, &stem);
        omf_free(settings_get()->tournament.last_name);
        settings_get()->tournament.last_name = omf_strdup(str_c(&stem));
        str_free(&stem);
        settings_save();
    }
    return ret;
}

int sg_delete(const char *pilot_name) {
    path save = get_save_directory();
    path_append(&save, pilot_name);
    path_set_ext(&save, ".CHR");
    path_dossify_filename(&save);

    const int ret = remove(path_c(&save));
    if(ret != 0) {
        log_error("Failed to delete %s: %s", path_c(&save), strerror(errno));
    } else {
        log_info("Removed %s", path_c(&save));
    }
    return ret;
}
