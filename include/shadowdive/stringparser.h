#ifndef _STRINGPARSER_H
#define _STRINGPARSER_H

typedef void(*sd_stringparser_cb_t)(int param);

enum {
    SD_BLEND_ADDITIVE = 0,
    SD_BLEND_ALPHA
};

typedef struct sd_stringparser_cbs_t {
    sd_stringparser_cb_t play_music;
    sd_stringparser_cb_t play_sound; 
} sd_stringparser_cbs;

typedef void* sd_tag_list;

typedef struct sd_stringparser_t {
    char *string;
    sd_stringparser_cbs cbs;
    sd_tag_list tag_list;

    int blendmode;
    int flip_horizontal;
    int flip_vertical;
} sd_stringparser;

sd_stringparser* sd_stringparser_create();
void sd_stringparser_delete(sd_stringparser *parser);
void sd_stringparser_set_string(sd_stringparser *parser, const char *string);
void sd_stringparser_set_cbs(sd_stringparser *parser, sd_stringparser_cbs callbacks);

void sd_stringparser_reset(sd_stringparser *parser);
int sd_stringparser_run(sd_stringparser *parser, unsigned long *ticks);

#endif // _STRINGPARSER_H
