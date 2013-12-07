#ifndef _SD_ALTPAL_H
#define _SD_ALTPAL_H

typedef struct sd_altpal_file_t {
    sd_palette palettes[11];
} sd_altpal_file;


sd_altpal_file* sd_altpal_create();
int sd_altpals_load(sd_altpal_file *ap, const char *filename);
int sd_altpals_save(sd_altpal_file *ap, const char *filename);
void sd_altpal_delete(sd_altpal_file *ap);

#endif // _SD_ALTPAL_H
