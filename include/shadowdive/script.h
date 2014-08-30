#ifndef _SD_SCRIPT_H
#define _SD_SCRIPT_H

#include "shadowdive/taglist.h"

typedef struct {
    const char* key;
    const char* desc;
    int has_param;
    int value;
} sd_script_tag;

typedef struct {
    int sprite;
    int tick_len;
    int tag_count;
    sd_script_tag *tags;
} sd_script_frame;

typedef struct {
    int total_ticks;
    int frame_count;
    sd_script_frame *frames;
} sd_script;

int sd_script_create(sd_script *script);
void sd_script_free(sd_script *script);
int sd_script_decode(sd_script *script, const char* str);
int sd_script_encode(const sd_script *script, char* str);
int sd_script_encoded_length(const sd_script *script);

sd_script_frame* sd_script_get_frame_at(const sd_script *script, int ticks);
sd_script_frame* sd_script_get_frame(const sd_script *script, int frame_number);
sd_script_tag* sd_script_get_tag(const sd_script_frame* frame, const char* tag);
int sd_script_isset(sd_script_frame *frame, const char* tag);
int sd_script_get(sd_script_frame *frame, const char* tag);

#endif // _SD_SCRIPT_H