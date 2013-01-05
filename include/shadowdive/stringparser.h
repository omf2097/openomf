#ifndef _STRINGPARSER_H
#define _STRINGPARSER_H

/* Tags for callbacks */
#define SD_CB_MUSIC "smo"
#define SD_CB_SOUND "s"

typedef void* sd_tag_list;
typedef void* sd_frame_list;
typedef void* sd_anim_frame;

/* sd_anim_frame is an opaque pointer that can by callbacks to get 
   information about the current frame and also set various flags */
typedef void(*sd_stringparser_cb_t)(sd_anim_frame frame, void *data, int param);

enum {
    SD_BLEND_ADDITIVE = 0,
    SD_BLEND_ALPHA
};

typedef struct sd_stringparser_t {
    char *string;
    sd_tag_list tag_list;
    sd_frame_list frame_list;

    int blendmode;
    int flip_horizontal;
    int flip_vertical;
} sd_stringparser;

sd_stringparser* sd_stringparser_create();
void sd_stringparser_delete(sd_stringparser *parser);

/* Parses the string and construct an animation list internally, may return error */
int sd_stringparser_set_string(sd_stringparser *parser, const char *string);

/* Set a callback, the data pointer is passed to the callback */
void sd_stringparser_set_cb(sd_stringparser *parser, const char *tag, sd_stringparser_cb_t callbacks, void *data);

/* Reset the animation to the first frame */
void sd_stringparser_reset(sd_stringparser *parser);

/* Run the animation at "ticks", may return error */
int sd_stringparser_run(sd_stringparser *parser, unsigned int ticks);


#endif // _STRINGPARSER_H
