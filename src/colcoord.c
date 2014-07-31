#include "shadowdive/error.h"
#include "shadowdive/colcoord.h"

int sd_col_coord_create(col_coord *coord) {
    coord->x = 0;
    coord->y = 0;
    coord->frame_id = 0;
    return SD_SUCCESS;
}

void sd_col_coord_free(col_coord *coord) {}