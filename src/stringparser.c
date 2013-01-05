#include "shadowdive/stringparser.h"
#include "shadowdive/error.h"

#define _BSD_SOURCE // for strdup
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

// private structs
typedef struct tag_attribute_t {
    const char *tag;
    int has_param;
    sd_stringparser_cb_t callback;
    void *data;
} tag_attribute;

typedef struct tag_attribute_init_t {
    const char *tag;
    int has_param;
} tag_attribute_init;

typedef struct tag_list_t {
    // traverse tag_chain to find whether a given tag is valid or not
    struct tag_list_t *tag_chain[256];
    tag_attribute attrib;
} tag_list;

typedef struct anim_frame_t {
    // is_done is set to 1 if this frame is completed also no further processing for this frame 
    int is_done;

    // the duration of this frame in "ticks"
    int duration;

    // frame letter is always uppercase
    char frame_letter;

    // a list of tags ready to be "interpreted"
    int num_tags;
    const char **tags;
    int *tag_params;
} anim_frame;

typedef struct frame_list_t {
    int num_frames;
    anim_frame *frames;

    int current_frame;
} frame_list;

enum {
    TAG_TAG=0,
    TAG_FRAME,
    TAG_MARKER,
    TAG_END
};

// list of valid tags and whether it has param or not
const tag_attribute_init tags[] = {
    {"aa", 0},
    {"ab", 0},
    {"ac", 0},
    {"ad", 0},
    {"ae", 0}, 
    {"af", 0},
    {"ag", 0},
    {"ai", 0},
    {"am", 0},
    {"ao", 0},
    {"as", 0},
    {"at", 0},
    {"aw", 0},
    {"ax", 0},
    {"ar", 0},
    {"al", 0},
         
    {"b1", 0},
    {"b2", 0},
    {"bb", 1},
    {"be", 0},
    {"bf", 1},
    {"bh", 0},
    {"bl", 1},
    {"bm", 1},
    {"bj", 1},
    {"bs", 1},
    {"bu", 0},
    {"bw", 0},
    {"bx", 0},

    
    {"bpd", 1},
    {"bps", 1},
    {"bpn", 1},
    {"bpf", 0},
    {"bpp", 1},
    {"bpb", 1},
    {"bpo", 0},
    {"bz",  0},

    {"ba", 1},
    {"bc", 1},
    {"bd", 0},
    {"bg", 0},
    {"bi", 1},
    {"bk", 1},
    {"bn", 0},
    {"bo", 1},
    {"br", 0},
    {"bt", 0},
    {"by", 0},

    {"cf", 0},
    {"cg", 0},
    {"cl", 0},
    {"cp", 0},
    {"cw", 0},
    {"cx", 1},
    {"cy", 1},

    {"d", 1},
    {"e", 0},
    {"f", 0},
    {"g", 0},
    {"h", 0},
    {"i", 0},

    {"jf2", 0},
    {"jf", 0},
    {"jg", 0},
    {"jh", 0},
    {"jj", 0},
    {"jl", 0},
    {"jm", 0},
    {"jp", 0},
    {"jz", 0},
    {"jn", 1},

    {"k",   1},
    {"l",   1},
    {"ma",  1},
    {"mc",  0},
    {"md",  1},
    {"mg",  1},
    {"mi",  1},
    {"mm",  1},
    {"mn",  1},
    {"mo",  0},
    {"mp",  1},
    {"mrx", 1},
    {"mry", 1},
    {"ms",  0},
    {"mu",  1},
    {"mx",  1},
    {"my",  1},
    {"m",   1},
    {"n",   0},
    {"ox",  1},
    {"oy",  1},
    {"pa",  0},
    {"pb",  1},
    {"pc",  1},
    {"pd",  1},
    {"pe",  0},
    {"ph",  0},
    {"pp",  1},
    {"ps",  0},
    {"ptd", 1},
    {"ptp", 1},
    {"ptr", 1},
    {"q",   0},
    {"r",   0},
    {"s",   1},
    {"sa",  0},
    {"sb",  1},
    {"sc",  1},
    {"sd",  0},
    {"se",  1},
    {"sf",  1},
    {"sl",  1},
    {"smf", 1},
    {"smo", 1},
    
    {"sp",  1},
    {"sw",  1},
    {"t",   0},
    {"ua",  0},
    {"ub",  0},
    {"uc",  0},
    {"ud",  0},
    {"ue",  0},
    {"uf",  0},
    {"ug",  0},
    {"uh",  0},
    {"uj",  0},
    {"ul",  0},
    {"un",  0},
    {"ur",  0},
    {"us",  0},
    {"uz",  0},
    {"v",   0},
    {"vsx", 0},
    {"vsy", 0},
    {"w",   0},
    
    {"x-", 1},
    {"x+", 1},
    {"x=", 1},
    {"x",  1}, // if unspecified a value of 100 is assumed
    
    {"y-", 1},
    {"y+", 1},
    {"y=", 1},
    {"y",  1}, // if unspecified a value of 100 is assumed
    
    {"zg", 0},
    {"zh", 0},
    {"zj", 0},
    {"zl", 0},
    {"zm", 0},
    {"zp", 0},
    {"zz", 0}
};

static void sd_taglist_add_tag(tag_list *list, const tag_attribute_init *attrib) {
    tag_list **plist = &list;
    const char *ptag = attrib->tag;
    do {
        if(*plist == NULL) {
            *plist = malloc(sizeof(tag_list));
            memset(*plist, 0, sizeof(tag_list));
        }
        plist = &((*plist)->tag_chain[(unsigned char)*ptag]);
    } while(*(++ptag));
    if(*plist == NULL) {
        *plist = malloc(sizeof(tag_list));
        memset(*plist, 0, sizeof(tag_list));
    }
    (*plist)->attrib.tag = attrib->tag;
    (*plist)->attrib.has_param = attrib->has_param;
}

static tag_attribute *sd_taglist_find_tag(tag_list *list, const char *tag) {
    tag_list **plist = &list;
    const char *ptag = tag;
    do {
        plist = &((*plist)->tag_chain[(unsigned char)*ptag]);
    } while(*(++ptag));

    if(*plist && strcmp((*plist)->attrib.tag, tag) == 0) {
        return &(*plist)->attrib;
    }

    return 0;
}

static void sd_taglist_init(tag_list *list) {
    for(int i = 0;i < sizeof(tags)/sizeof(tag_attribute_init);++i) {
        sd_taglist_add_tag(list, tags + i);
    }
}

static void sd_taglist_clear(tag_list *list) {
    for(int i = 0;i < sizeof(list->tag_chain)/sizeof(tag_list);++i) {
        if(list->tag_chain[i]) {
            sd_taglist_clear(list->tag_chain[i]);
            list->tag_chain[i] = NULL;
        }
    }
}

static void sd_taglist_set_cb(tag_list *list, const char *tag, sd_stringparser_cb_t cb, void *data) {
    if(tag == NULL || cb == NULL) return;

    tag_attribute *tag_attrib = sd_taglist_find_tag(list, tag);

    if(tag_attrib) {
        tag_attrib->callback = cb;
        tag_attrib->data = data;
    }
}

static void sd_framelist_clear(frame_list *list) {
    for (int i = 0;i < list->num_frames;++i)
    {
        anim_frame *f = &list->frames[i];
        free(f->tag_params);
        free(f->tags);
        memset(f, 0, sizeof(anim_frame));
    }
    free(list->frames);
    list->frames = NULL;
    list->current_frame = 0;
    list->num_frames = 0;
}

static void sd_framelist_set(frame_list *list, int cur_frame, char frame_letter, int duration) {
    anim_frame *frame = &list->frames[cur_frame];
    frame->frame_letter = frame_letter;
    frame->duration = duration;
}

static void sd_framelist_add_tag(frame_list *list, int cur_frame, tag_attribute *tag_attrib, int param){
    anim_frame *cur = &list->frames[cur_frame];
    cur->num_tags++;
    int ntags = cur->num_tags;

    cur->tag_params = realloc(cur->tag_params, ntags*sizeof(int));
    cur->tags = realloc(cur->tags, ntags*sizeof(char*));

    cur->tag_params[ntags-1] = param;
    cur->tags[ntags-1] = tag_attrib->tag;
}

static void sd_framelist_resize(frame_list *list, int frames) {
    int prev_num_frames = list->num_frames;
    list->num_frames = frames;
    if(frames > 0) {
        list->frames = realloc(list->frames, frames*sizeof(anim_frame));
        if(frames > prev_num_frames) memset(list->frames+prev_num_frames, 0, (frames-prev_num_frames)*sizeof(anim_frame));
    } else {
        free(list->frames);
        list->frames = NULL;
    }
    if(list->current_frame > frames) list->current_frame = frames-1;
}

static void sd_framelist_process(frame_list *frames, tag_list *tags, unsigned int ticks) {
    unsigned int next_frame_time=0;
    for(int i = 0;i < frames->num_frames;++i) {
        anim_frame *cur = &frames->frames[i];
        next_frame_time += cur->duration;
        if(ticks < next_frame_time) {
            // we are in the current frame
            if(!cur->is_done) {
                for(int i = 0;i < cur->num_tags;++i) {
                    tag_attribute *tag = sd_taglist_find_tag(tags, cur->tags[i]);
                    if(tag->callback) tag->callback(cur, tag->data, cur->tag_params[i]);
                }
                
            }
        }
    }
}

/* Reads next integer value from string (eg md15s5- -> reads "15" and leaves the position to point to 's'. */
int rn_int(int *pos, const char *str) {
    int opos = 0;
    char buf[20];
    memset(buf, 0, 20);
    if (str[*pos] == '-' && str[(*pos)+1] >= '0' && str[(*pos)+1] <= '9') {
        // begins with - and is followed by a digit, must be negative number
        buf[opos] = str[*pos];
        (*pos)++;
        opos++;
    }
    while(str[*pos] >= '0' && str[*pos] <= '9') {
        buf[opos] = str[*pos];
        (*pos)++;
        opos++;
    }

    if(opos == 0) return 0;
    return atoi(buf);
}

// Reads frame letter and duration
void rn_frame(const char **str, char *frame_letter, int *duration) {
    if(isupper(**str)) {
        *frame_letter = **str;
        (*str)++;
    } else { 
        frame_letter = 0;
    }

    int pos=0;
    *duration = rn_int(&pos, *str);
    *str += pos;
}

// returns 0 if found, otherwise return nonzero
int rn_tag_attribute(tag_list *list, const char **str, tag_attribute *attrib) {
    const int N_LOOK = 3;
    const tag_attribute *scanned[N_LOOK];
    int nscanned = 0;
    tag_list *cur=list->tag_chain[(unsigned char)**str];

    memset(scanned, 0, sizeof(scanned));

    do {
        if(cur == NULL) {
            break;
        } else {
            scanned[nscanned] = &cur->attrib;
            nscanned++;
            if(nscanned == N_LOOK) {
                break;
            }
        }
        ++(*str);
        cur = cur->tag_chain[(unsigned char)**str];
    } while(**str);

    const char *skipto = *str;
    memset(attrib, 0, sizeof(tag_attribute));
    for(nscanned--;nscanned >= 0;--nscanned, --(*str)) {
        if(scanned[nscanned]->tag) {
            *attrib = *scanned[nscanned];
            return 0;
        }
    }
    // skip current tag if its not found
    *str = skipto;

    return 1;
}
void rn_descriptor_marker(const char **str) {
    if(**str == '-') (*str)++;
}

// skip to the next tag or frame
int next_tag(const char **str) {
    do {
        if(islower(**str)) {
            return TAG_TAG;
        } else if(isupper(**str)) {
            return TAG_FRAME;
        } else if(**str == '-' && isupper(*((*str)+1))) {
            return TAG_MARKER;
        }
    } while(*((*str)++));

    (*str)--;
    return TAG_END;
}

void parse_string(sd_stringparser *parser, 
                  void(*frame_cb)(sd_stringparser*, void*, char,int), 
                  void(*tag_cb)(sd_stringparser*, void*, tag_attribute*, int param), 
                  void *data) {

    if(parser->string) {
        int end_of_frame = 0;
        int end_of_stream = 0;
        int pos = 0;
        while(1) {
            const char *start = parser->string + pos;
            const char *str = parser->string + pos;

            int type = next_tag(&str);
            pos += (str-start);
            start = parser->string + pos;

            if(type == TAG_TAG) {
                // a tag
                tag_attribute attrib;
                if(rn_tag_attribute(parser->tag_list, &str, &attrib) == 0) {
                    // read the numeric param and call the callback function
                    // if a param is not present, 0 is assumed
                    int pos = 0;
                    int param = rn_int(&pos, str); 
                    str += pos;
                    if(tag_cb) tag_cb(parser, data, &attrib, param);
                }
            } else if(type == TAG_FRAME) {
                // a frame
                int duration=0;
                char frame_letter=0;
                rn_frame(&str, &frame_letter, &duration);

                if(frame_cb) frame_cb(parser, data, frame_letter, duration);
            } else if(type == TAG_MARKER) {
                // an end of frame descriptor marker (a dash, '-')
                rn_descriptor_marker(&str);
                end_of_frame = 1;
            } else {
                // the end of stream
                end_of_stream = 1;
                break;
            }

            pos += (str-start);
            if(end_of_frame) break;
        }
        if(end_of_frame) {

        }
        if(end_of_stream) {

        }
    }
}

void cb_count_frame(sd_stringparser *parser, void *pframes, char frame_letter, int duration) {
    int *frames = pframes;
    (*frames)++;
}
void cb_set_frame(sd_stringparser *parser, void *pcur_frame, char frame_letter, int duration) {
    int *cur_frame = pcur_frame;
    sd_framelist_set(parser->frame_list, *cur_frame, frame_letter, duration);
    (*cur_frame)++;
}

void cb_store_tag(sd_stringparser *parser, void *pcur_frame, tag_attribute *tag_attrib, int param) {
    int *cur_frame = pcur_frame;
    sd_framelist_add_tag(parser->frame_list, *cur_frame, tag_attrib, param);
}

sd_stringparser* sd_stringparser_create() {
    sd_stringparser *parser = (sd_stringparser*)malloc(sizeof(sd_stringparser));
    parser->tag_list = malloc(sizeof(tag_list));
    memset(parser->tag_list, 0 , sizeof(tag_list));
    parser->frame_list = malloc(sizeof(frame_list));
    memset(parser->frame_list, 0 , sizeof(frame_list));
    parser->string = 0;
    sd_taglist_init(parser->tag_list);
    sd_stringparser_reset(parser);
    return parser;
}

void sd_stringparser_delete(sd_stringparser *parser) {
    if(parser) {
        if(parser->string) free(parser->string);
        sd_taglist_clear(parser->tag_list);
        sd_framelist_clear(parser->frame_list);
        free(parser->frame_list);
        free(parser->tag_list);
        free(parser);
    }
}

int sd_stringparser_set_string(sd_stringparser *parser, const char *string) {
    if(parser->string) free(parser->string);
    parser->string = strdup(string);

    int frames=0;
    parse_string(parser, cb_count_frame, NULL, &frames);
    sd_framelist_resize(parser->frame_list, frames);

    // ignore animation string that does not contain frame letters
    if(frames > 0) {
        frames = 0;
        parse_string(parser, cb_set_frame, NULL, &frames);

        // now store the tags and params into the frame struct
        frames = 0;
        parse_string(parser, cb_count_frame, cb_store_tag, &frames);
        return 0;
    }
    return SD_ANIM_INVALID_STRING;
}

void sd_stringparser_set_cb(sd_stringparser *parser, const char *tag, sd_stringparser_cb_t cb, void *data) {
    sd_taglist_set_cb(parser->tag_list, tag, cb, data);
}

void sd_stringparser_reset(sd_stringparser *parser) {
    parser->blendmode = 0;
    parser->flip_horizontal = 0;
    parser->flip_vertical = 0;
}


int sd_stringparser_run(sd_stringparser *parser, unsigned int ticks) {
    // Right, so
    // 1. Jump to the part of the string that "ticks" variable points to
    // 2. If the part has already been handled, just return.
    //    If not, parse the string part, call callbacks, etc.
    // 3. Return 0 for success, != 0 for somekind of error (use sd_error for returning string representation)

    sd_framelist_process(parser->frame_list, parser->tag_list, ticks);

    return 0;
}
