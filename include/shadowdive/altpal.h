#ifndef _SD_ALTPAL_H
#define _SD_ALTPAL_H

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    sd_palette palettes[11];
} sd_altpal_file;

int sd_altpal_create(sd_altpal_file *ap);
int sd_altpals_load(sd_altpal_file *ap, const char *filename);
int sd_altpals_save(sd_altpal_file *ap, const char *filename);
void sd_altpal_free(sd_altpal_file *ap);

#ifdef __cplusplus
}
#endif

#endif // _SD_ALTPAL_H
