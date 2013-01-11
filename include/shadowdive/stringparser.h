#ifndef _SD_STRINGPARSER_H
#define _SD_STRINGPARSER_H

#include "stringparser_tags.h"

typedef void* sd_stringparser_tag_list;
typedef void* sd_stringparser_frame_list;

typedef struct sd_stringparser_tag_info_t {
    const char *tag;
    const int has_param;
    const char *description;
} sd_stringparser_tag_info;

typedef struct sd_stringparser_cb_param_t {
    // tag_info and tag_value are unavailable during frame change callback
    const sd_stringparser_tag_info *tag_info;
    const int tag_value;

    // a list of tags for the current frame
    const int num_tags;
    const char **tags;
    const int *tag_values;

    /* The current tick */
    const int tick;

    // the duration of this frame
    const int duration;

    // the frame character in uppercase
    const char frame;
    
    /* The userdata pointer that was passed to sd_stringparser_set_default_cb/sd_stringparser_set_cb */
    void *userdata;
} sd_stringparser_cb_param;

typedef void(*sd_stringparser_cb_t)(sd_stringparser_cb_param *info);

typedef struct sd_stringparser_t {
    char *string;
    sd_stringparser_tag_list tag_list;
    sd_stringparser_frame_list frame_list;
} sd_stringparser;

sd_stringparser* sd_stringparser_create();
void sd_stringparser_delete(sd_stringparser *parser);

/* Parses the string and construct an animation list internally, may return error */
int sd_stringparser_set_string(sd_stringparser *parser, const char *string);

/* Set a callback, the userdata pointer is passed to the callback */
void sd_stringparser_set_cb(sd_stringparser *parser, const char *tag, sd_stringparser_cb_t callback, void *userdata);

/* Set a default callback to handle every other tags */
void sd_stringparser_set_default_cb(sd_stringparser *parser, sd_stringparser_cb_t callback, void *userdata);

/* Set a callback to gets called when changing to a new frame */
void sd_stringparser_set_frame_change_cb(sd_stringparser *parser, sd_stringparser_cb_t callback, void *userdata);

/* Reset the animation to the first frame */
void sd_stringparser_reset(sd_stringparser *parser);

/* Run the animation at "ticks", may return error */
int sd_stringparser_run(sd_stringparser *parser, unsigned int ticks);

int sd_stringparser_prettyprint_frame(sd_stringparser *parser, unsigned int frame);

int sd_stringparser_prettyprint(sd_stringparser *parser);


#endif // _SD_STRINGPARSER_H
