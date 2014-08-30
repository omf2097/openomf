#ifndef _SD_TAGLIST_H
#define _SD_TAGLIST_H

typedef struct {
    const char *tag;
    const int has_param;
    const char *description;
} sd_tag;

extern const sd_tag sd_taglist[];
extern const int sd_taglist_size;

int sd_tag_info(const char* search_tag, int *req_param, const char **tag, const char **desc);

#endif // _SD_TAGLIST_H
