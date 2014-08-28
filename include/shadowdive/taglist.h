#ifndef _SD_TAGLIST_H
#define _SD_TAGLIST_H

typedef struct {
    const char *tag;
    const int has_param;
    const char *description;
} sd_tag;

extern const sd_tag sd_taglist[];

#endif // _SD_TAGLIST_H
