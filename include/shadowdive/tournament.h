#ifndef _TOURNAMENT_H
#define _TOURNAMENT_H

#include <stdint.h>

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct sd_tournament_file_t {
    int id;
} sd_tournament_file;

sd_tournament_file* sd_tournament_create();
int sd_tournament_load(sd_tournament_file *trn, const char *filename);
int sd_tournament_save(sd_tournament_file *trn, const char *filename);
void sd_tournament_delete(sd_tournament_file *trn);

#ifdef __cplusplus
}
#endif

#endif // _TOURNAMENT_H
