#include "resources/bk_loader.h"
#include "formats/bk.h"
#include "formats/error.h"

int load_bk_file(bk *b, const path *filename) {
    // Load up BK file from libSD
    sd_bk_file tmp;
    if(sd_bk_create(&tmp) != SD_SUCCESS) {
        return 1;
    }
    if(sd_bk_load(&tmp, filename) != SD_SUCCESS) {
        sd_bk_free(&tmp);
        return 1;
    }

    str fn;
    path_stem(filename, &fn);

    // Convert
    bk_create(b, &tmp, &fn);
    sd_bk_free(&tmp);
    return 0;
}
