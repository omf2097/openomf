#include "resources/modmanager.h"

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

int mod_count(void) {
    list dir_list;
    list_create(&dir_list);
    const path scan = get_system_mod_directory();
    if(!path_glob(&scan, &dir_list, "*.zip")) {
        log_warn("Failed to scan system mods directory!");
    } else {
        log_info("Found %d system mods.", list_size(&dir_list));
    }
    scan = get_user_mod_directory();
    if(!path_glob(&scan, &dir_list, "*.zip")) {
        log_warn("Failed to scan user mods directory!");
    } else {
        log_info("Found %d user mods.", list_size(&dir_list));
    }
    size += list_size(&dir_list);
    list_free(&dir_list);

    return size;
}
