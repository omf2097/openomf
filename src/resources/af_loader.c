#include "resources/af_loader.h"
#include "resources/pathmanager.h"
#include "formats/af.h"
#include "formats/error.h"

int load_af_file(af *a, int id) {
    // Get directory + filename
    const char *filename = pm_get_resource_path(id);

    // Load up AF file from libSD
    sd_af_file tmp;
    if(sd_af_create(&tmp) != SD_SUCCESS) {
        return 1;
    }
    if(sd_af_load(&tmp, filename) != SD_SUCCESS) {
        sd_af_free(&tmp);
        return 1;
    }

    // Convert
    af_create(a, &tmp);
    sd_af_free(&tmp);
    return 0;
}
