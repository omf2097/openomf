#ifndef _SD_REC_H
#define _SD_REC_H

#include <stdint.h>
#include "shadowdive/pilot.h"

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    sd_pilot *pilots[2];
    char unknown[32];
    char *raw;
    size_t rawsize;
} sd_rec_file;

int sd_rec_create(sd_rec_file *rec);
void sd_rec_free(sd_rec_file *rec);
int sd_rec_load(sd_rec_file *rec, const char *file);
int sd_rec_save(sd_rec_file *rec, const char *file);

#ifdef __cplusplus
}
#endif

#endif // _SD_REC_H
