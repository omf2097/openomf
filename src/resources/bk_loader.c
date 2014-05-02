#include "resources/bk_loader.h"
#include "resources/pathmanager.h"
#include <shadowdive/shadowdive.h>

int load_bk_file(bk *b, int id) {
    // Get directory + filename
    const char *filename = pm_get_resource_path(id);

    // Load up BK file from libSD
    sd_bk_file *tmp = sd_bk_create();
    if(sd_bk_load(tmp, filename) != SD_SUCCESS) {
        sd_bk_delete(tmp);
        return 1;
    }

    // Convert
    bk_create(b, tmp);
    sd_bk_delete(tmp);
    return 0;
}
