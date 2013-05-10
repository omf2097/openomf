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

typedef struct sd_stringparser_tag_value {
    int is_set;
    int value;
} sd_stringparser_tag_value;

typedef struct sd_stringparser_frame_t {
    /* The owner of this frame */
    struct sd_stringparser_t *parser;

    /* Zero-based unique id for this frame */
    int id;

    /* a list of tags for the current frame */
    int num_tags;
    const char **tags;
    int *tag_values;

    /* the duration of this frame */
    int duration;

    /* the frame character in uppercase */
    char letter;

    /* is_first_frame is set to 1 if the current frame is the first frame of this animation */
    int is_first_frame;

    /* is_final_frame is set to 1 if the current frame is the final frame of this animation */
    int is_final_frame;

    /* is_animation_end is set to 1 if the current animation has ended (ie. 1 frame after the final frame) */
    int is_animation_end;
} sd_stringparser_frame;

typedef struct sd_stringparser_t {
    char *string;
    sd_stringparser_tag_list tag_list;
    sd_stringparser_frame_list frame_list;
    
    /* current_frame is not available until sd_stringparser_run has been called first */
    sd_stringparser_frame current_frame;
} sd_stringparser;

sd_stringparser* sd_stringparser_create();
void sd_stringparser_delete(sd_stringparser *parser);

/* Parses the string and construct an animation list internally, may return error */
int sd_stringparser_set_string(sd_stringparser *parser, const char *string);

/* Reset the animation to the first frame */
void sd_stringparser_reset(sd_stringparser *parser);

/* Run the animation at "ticks", may return error */
/* ticks must be externally incremented/decremented */
/* An increment/decrement of ticks greater than 1 will trigger a frame jump */
int sd_stringparser_run(sd_stringparser *parser, unsigned int ticks);

/* Run the animation from ticks A to B, may return error */
/* ticks must be externally incremented/decremented */
/* An increment/decrement of ticks greater than 1 will trigger a frame jump */
int sd_stringparser_run_ex(sd_stringparser *parser, unsigned int ticks, unsigned int end_ticks);

/* Run the animation until the end of specified frame, may return error */
/* ticks must be externally incremented/decremented */
/* An increment/decrement of ticks greater than 1 will trigger a frame jump */
int sd_stringparser_run_frames(sd_stringparser *parser, unsigned int ticks, unsigned int end_frame);

/* Jump to frame, the starting tick of the frame is stored in ticks, may return error */
int sd_stringparser_goto_frame(sd_stringparser *parser, unsigned int frame, unsigned int *ticks);

int sd_stringparser_peek(sd_stringparser *parser, unsigned int frame, sd_stringparser_frame *out_frame);

/* Return 0 if the tag was found, otherwise return 1 */
/* out_tag must be declared as const sd_stringparser_tag_value* */
int sd_stringparser_get_tag(sd_stringparser *parser, unsigned int frame, const char *tag, const sd_stringparser_tag_value **out_tag);

/* Return 0 if the tag was found, otherwise return 1 */
/* out_tag must be declared as const sd_stringparser_tag_info* */
int sd_stringparser_get_taginfo(sd_stringparser *parser, const char *tag, const sd_stringparser_tag_info ** out_info);

int sd_stringparser_prettyprint_frame(sd_stringparser *parser, unsigned int frame);

int sd_stringparser_prettyprint(sd_stringparser *parser);

int sd_stringparser_get_current_frame_id(sd_stringparser *parser);

char sd_stringparser_get_current_frame_letter(sd_stringparser *parser);

#endif // _SD_STRINGPARSER_H
