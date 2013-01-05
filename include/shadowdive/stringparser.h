#ifndef _STRINGPARSER_H
#define _STRINGPARSER_H

enum {
    SD_BLEND_ADDITIVE = 0,
    SD_BLEND_ALPHA
};

typedef struct sd_stringparser_cbs_t {
    void (*play_music)(int id); 
    void (*play_sound)(int id); 
} sd_stringparser_cbs;

typedef struct sd_stringparser_t {
    char *string;
    sd_stringparser_cbs cbs;
    
    int blendmode;
    int flip_horizontal;
    int flip_vertical;
} sd_stringparser;

sd_stringparser* sd_stringparser_create();
void sd_stringparser_delete(sd_stringparser *parser);
void sd_stringparser_set_cbs(sd_stringparser *parser, sd_stringparser_cbs callbacks);

void sd_stringparser_reset(sd_stringparser *parser);
int sd_stringparser_run(sd_stringparser *parser, unsigned long ticks);

#endif // _STRINGPARSER_H