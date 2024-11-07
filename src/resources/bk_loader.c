#include "resources/bk_loader.h"
#include "formats/bk.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"

int load_bk_file(bk *b, int id) {
    // Get directory + filename
    const char *filename = pm_get_resource_path(id);

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

int bk_inc_create(bk_inc *b, int id) {
    // Get directory + filename
    const char *filename = pm_get_resource_path(id);

    // Initialize reader
    if(!(b->r = sd_reader_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    if(sd_bk_create(&b->sd_bk) != SD_SUCCESS) {
        sd_bk_free(&b->sd_bk);
        sd_reader_close(b->r);
        return 1;
    }

    b->state = 1;
    return SD_AGAIN;
}

int load_bk_file_incremental(bk_inc *b, int id) {
    int ret = SD_SUCCESS;
    switch(b->state) {
        case 0:
            return bk_inc_create(b, id);
        case 1:
            ret = sd_bk_load_incremental(&b->sd_bk, b->r);
            if(ret == SD_SUCCESS) {
                b->state = 2;
                return SD_AGAIN;
            }
            if(ret != SD_AGAIN) {
                sd_reader_close(b->r);
            }
            return ret;
        case 2:
            b->bk = omf_calloc(1, sizeof(bk));
            DEBUG("creating BK %p", b->bk);
            // this should be fast, so do it in one pass
            bk_create(b->bk, &b->sd_bk);
            sd_bk_free(&b->sd_bk);
            sd_reader_close(b->r);
            b->state = 3;
            return SD_SUCCESS;
    }
    return ret;
}
