#ifndef _SD_CHR_H
#define _SD_CHR_H

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct sd_chr_file_t {
    int tmp;
} sd_chr_file;

sd_chr_file* sd_chr_create();
int sd_chr_load(sd_chr_file *chr, const char *filename);
int sd_chr_save(sd_chr_file *chr, const char *filename);
void sd_chr_delete(sd_chr_file *chr);

#ifdef __cplusplus
}
#endif

#endif // _SD_CHR_H