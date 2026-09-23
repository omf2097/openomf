#include "archipelago/ap_sgmanager.h"

#include "formats/error.h"
#include "game/utils/settings.h"
#include "resources/resource_files.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include <errno.h>
#include <string.h>

path get_ap_save_directory(void) {
    path name = get_save_directory();
    path_append(&name, "ap");
    if(!path_exists(&name)) path_mkdir(&name);
    return name;
}

static int save_pilot_to_dir(sd_chr_file *chr, path dir, const char *filename_stem) {
    path_append(&dir, filename_stem);
    path_set_ext(&dir, ".CHR");
    path_dossify_filename(&dir);
    int ret = sd_chr_save(chr, &dir);
    if(ret != SD_SUCCESS) {
        log_error("Saving pilot to %s failed: %s", path_c(&dir), strerror(errno));
        return ret;
    }
    log_info("Saved pilot %s to %s", str_c(&chr->pilot.name), path_c(&dir));
    str stem;
    path_stem(&dir, &stem);
    omf_free(settings_get()->tournament.last_name);
    settings_get()->tournament.last_name = omf_strdup(str_c(&stem));
    str_free(&stem);
    settings_save();
    return ret;
}

int sg_save_as(sd_chr_file *chr, const char *filename_stem) {
    return save_pilot_to_dir(chr, get_save_directory(), filename_stem);
}

int sg_save_ap(sd_chr_file *chr, const char *filename_stem) {
    return save_pilot_to_dir(chr, get_ap_save_directory(), filename_stem);
}

int sg_load_ap_pilot(sd_chr_file *chr, const char *pilot_name) {
    path save = get_ap_save_directory();
    path_append(&save, pilot_name);
    path_set_ext(&save, ".CHR");
    path_dossify_filename(&save);
    if(!path_is_file(&save)) return SD_FILE_OPEN_ERROR;
    return sg_load(chr, &save);
}
