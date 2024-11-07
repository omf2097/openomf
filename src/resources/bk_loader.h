#ifndef BK_LOADER_H
#define BK_LOADER_H

#include "formats/bk.h"
#include "formats/error.h"
#include "resources/bk.h"

/*typedef enum {
    BK_LOADER_INIT,
    BK_LOADER_LOAD_BK,*/

typedef struct {
    int id;
    sd_reader *r;
    int state;
    sd_bk_file sd_bk;
    bk *bk;
} bk_inc;

int load_bk_file(bk *b, int id);
int load_bk_file_incremental(bk_inc *b, int id);

#endif // BK_LOADER_H
