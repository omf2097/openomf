#include "resources/bk_loader.h"
#include "formats/bk.h"
#include "formats/error.h"
#include "resources/pathmanager.h"

int load_bk_file(bk *b, char const *filename) {
    // Load up BK file from libSD
    sd_bk_file tmp;
    if(sd_bk_create(&tmp) != SD_SUCCESS) {
        return 1;
    }
    if(sd_bk_load(&tmp, filename) != SD_SUCCESS) {
        sd_bk_free(&tmp);
        return 1;
    }

    // Convert
    bk_create(b, &tmp);
    sd_bk_free(&tmp);
    return 0;
}
