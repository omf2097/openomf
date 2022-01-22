#include <string.h>

#include "formats/error.h"
#include "formats/internal/reader.h"
#include "formats/setup.h"

int sd_setup_create(sd_setup_file *setup) {
    if(setup == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(setup, 0, sizeof(sd_setup_file));
    return SD_SUCCESS;
}

void sd_setup_free(sd_setup_file *setup) {
    if(setup == NULL)
        return;
}

int sd_setup_load(sd_setup_file *setup, const char *file) {
    int ret = SD_FILE_PARSE_ERROR;
    if(setup == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(file);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    if(sd_reader_filesize(r) != 296) {
        ret = SD_FILE_INVALID_TYPE;
        goto error_0;
    }

    if(!sd_read_buf(r, (char *)setup, 296)) {
        ret = SD_FILE_PARSE_ERROR;
        goto error_0;
    }

    sd_reader_close(r);
    return SD_SUCCESS;

error_0:
    sd_reader_close(r);
    return ret;
}
