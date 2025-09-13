#include "resources/af_loader.h"

#include "formats/af.h"
#include "formats/error.h"
#include "ids.h"
#include "resource_files.h"

int load_af_file(af *a, int id) {
    // Get directory + filename
    const path filename = get_resource_filename(get_resource_file(id));

    // Load up AF file from libSD
    sd_af_file tmp;
    if(sd_af_create(&tmp) != SD_SUCCESS) {
        return 1;
    }
    if(sd_af_load(&tmp, path_c(&filename)) != SD_SUCCESS) {
        sd_af_free(&tmp);
        return 1;
    }

    // Convert
    af_create(a, &tmp);
    sd_af_free(&tmp);
    return 0;
}
