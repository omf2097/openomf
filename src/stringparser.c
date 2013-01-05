#include "shadowdive/stringparser.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

// private struct
typedef struct tag_attribute_t {
    const char *tag;
    int has_param;
    sd_stringparser_cb_t callback;
} tag_attribute;

typedef struct tag_list_t {
    // traverse tag_chain to find whether a given tag is valid or not
    struct tag_list_t *tag_chain[255];
    tag_attribute attrib;
} tag_list;

enum {
    TAG_TAG=0,
    TAG_FRAME,
    TAG_MARKER,
    TAG_END
};

static tag_attribute tags[] = {
    {"aa", 0, NULL},
    {"ab", 0, NULL},
    {"ac", 0, NULL},
    {"ad", 0, NULL},
    {"ae", 0, NULL}, 
    {"af", 0, NULL},
    {"ag", 0, NULL},
    {"ai", 0, NULL},
    {"am", 0, NULL},
    {"ao", 0, NULL},
    {"as", 0, NULL},
    {"at", 0, NULL},
    {"aw", 0, NULL},
    {"ax", 0, NULL},
    {"ar", 0, NULL},
    {"al", 0, NULL},
         
    {"b1", 0, NULL},
    {"b2", 0, NULL},
    {"bb", 1, NULL},
    {"be", 0, NULL},
    {"bf", 1, NULL},
    {"bh", 0, NULL},
    {"bl", 1, NULL},
    {"bm", 1, NULL},
    {"bj", 1, NULL},
    {"bs", 1, NULL},
    {"bu", 0, NULL},
    {"bw", 0, NULL},
    {"bx", 0, NULL},

    
    {"bpd", 1, NULL},
    {"bps", 1, NULL},
    {"bpn", 1, NULL},
    {"bpf", 0, NULL},
    {"bpp", 1, NULL},
    {"bpb", 1, NULL},
    {"bpo", 0, NULL},
    {"bz",  0, NULL},

    {"ba", 1, NULL},
    {"bc", 1, NULL},
    {"bd", 0, NULL},
    {"bg", 0, NULL},
    {"bi", 1, NULL},
    {"bk", 1, NULL},
    {"bn", 0, NULL},
    {"bo", 1, NULL},
    {"br", 0, NULL},
    {"bt", 0, NULL},
    {"by", 0, NULL},

    {"cf", 0, NULL},
    {"cg", 0, NULL},
    {"cl", 0, NULL},
    {"cp", 0, NULL},
    {"cw", 0, NULL},
    {"cx", 1, NULL},
    {"cy", 1, NULL},

    {"d", 1, NULL},
    {"e", 0, NULL},
    {"f", 0, NULL},
    {"g", 0, NULL},
    {"h", 0, NULL},
    {"i", 0, NULL},

    {"jf2", 0, NULL},
    {"jf", 0, NULL},
    {"jg", 0, NULL},
    {"jh", 0, NULL},
    {"jj", 0, NULL},
    {"jl", 0, NULL},
    {"jm", 0, NULL},
    {"jp", 0, NULL},
    {"jz", 0, NULL},
    {"jn", 1, NULL},

    {"k",   1, NULL},
    {"l",   1, NULL},
    {"ma",  1, NULL},
    {"mc",  0, NULL},
    {"md",  1, NULL},
    {"mg",  1, NULL},
    {"mi",  1, NULL},
    {"mm",  1, NULL},
    {"mn",  1, NULL},
    {"mo",  0, NULL},
    {"mp",  1, NULL},
    {"mrx", 1, NULL},
    {"mry", 1, NULL},
    {"ms",  0, NULL},
    {"mu",  1, NULL},
    {"mx",  1, NULL},
    {"my",  1, NULL},
    {"m",   1, NULL},
    {"n",   0, NULL},
    {"ox",  1, NULL},
    {"oy",  1, NULL},
    {"pa",  0, NULL},
    {"pb",  1, NULL},
    {"pc",  1, NULL},
    {"pd",  1, NULL},
    {"pe",  0, NULL},
    {"ph",  0, NULL},
    {"pp",  1, NULL},
    {"ps",  0, NULL},
    {"ptd", 1, NULL},
    {"ptp", 1, NULL},
    {"ptr", 1, NULL},
    {"q",   0, NULL},
    {"r",   0, NULL},
    {"s",   1, NULL},
    {"sa",  0, NULL},
    {"sb",  1, NULL},
    {"sc",  1, NULL},
    {"sd",  0, NULL},
    {"se",  1, NULL},
    {"sf",  1, NULL},
    {"sl",  1, NULL},
    {"smf", 1, NULL},
    {"smo", 1, NULL},
    
    {"sp",  1, NULL},
    {"sw",  1, NULL},
    {"t",   0, NULL},
    {"ua",  0, NULL},
    {"ub",  0, NULL},
    {"uc",  0, NULL},
    {"ud",  0, NULL},
    {"ue",  0, NULL},
    {"uf",  0, NULL},
    {"ug",  0, NULL},
    {"uh",  0, NULL},
    {"uj",  0, NULL},
    {"ul",  0, NULL},
    {"un",  0, NULL},
    {"ur",  0, NULL},
    {"us",  0, NULL},
    {"uz",  0, NULL},
    {"v",   0, NULL},
    {"vsx", 0, NULL},
    {"vsy", 0, NULL},
    {"w",   0, NULL},
    
    {"x-", 1, NULL},
    {"x+", 1, NULL},
    {"x=", 1, NULL},
    {"x",  1, NULL}, // if unspecified a value of 100 is assumed
    
    {"y-", 1, NULL},
    {"y+", 1, NULL},
    {"y=", 1, NULL},
    {"y",  1, NULL}, // if unspecified a value of 100 is assumed
    
    {"zg", 0, NULL},
    {"zh", 0, NULL},
    {"zj", 0, NULL},
    {"zl", 0, NULL},
    {"zm", 0, NULL},
    {"zp", 0, NULL},
    {"zz", 0, NULL}
};

static void sd_taglist_add_tag(tag_list *list, const tag_attribute *attrib) {
    tag_list **plist = &list;
    const char *ptag = attrib->tag;
    do {
        if(*plist == NULL) {
            *plist = malloc(sizeof(tag_list));
            memset(*plist, 0, sizeof(tag_list));
        }
        plist = &((*plist)->tag_chain[*ptag]);
    } while(*(++ptag));
    if(*plist == NULL) {
        *plist = malloc(sizeof(tag_list));
        memset(*plist, 0, sizeof(tag_list));
    }
    (*plist)->attrib = *attrib;
}

static void sd_taglist_init(tag_list *list) {
    for(int i = 0;i < sizeof(tags)/sizeof(tag_attribute);++i) {
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

static void sd_taglist_set_cb(tag_list *list, const char *tag, sd_stringparser_cb_t cb) {
    if(tag == NULL || cb == NULL) return;

    tag_list **plist = &list;
    const char *ptag = tag;
    do {
        plist = &((*plist)->tag_chain[*ptag]);
    } while(*(++ptag));

    if(*plist && strcmp((*plist)->attrib.tag, tag) == 0) {
        (*plist)->attrib.callback = cb;
    }
}


sd_stringparser* sd_stringparser_create() {
    sd_stringparser *parser = (sd_stringparser*)malloc(sizeof(sd_stringparser));
    parser->tag_list = malloc(sizeof(tag_list));
    memset(parser->tag_list, 0 , sizeof(tag_list));
    parser->string = 0;
    sd_taglist_init(parser->tag_list);
    sd_stringparser_reset(parser);
    return parser;
}

void sd_stringparser_delete(sd_stringparser *parser) {
    if(parser) {
        if(parser->string) free(parser->string);
        sd_taglist_clear(parser->tag_list);
        free(parser->tag_list);
        free(parser);
    }
}
void sd_stringparser_set_string(sd_stringparser *parser, const char *string) {
    if(parser->string) free(parser->string);
    parser->string = strdup(string);
}

void sd_stringparser_set_cbs(sd_stringparser *parser, sd_stringparser_cbs cbs) {
    if(!parser) return;
    parser->cbs = cbs;
    sd_taglist_set_cb(parser->tag_list, "smo", cbs.play_music);
    sd_taglist_set_cb(parser->tag_list, "s",   cbs.play_sound);
}

void sd_stringparser_reset(sd_stringparser *parser) {
    if(parser->string) {
        free(parser->string);
        parser->string = 0;
    }
    parser->blendmode = 0;
    parser->flip_horizontal = 0;
    parser->flip_vertical = 0;
}

/* Reads next integer value from string (eg md15s5- -> reads "15" and leaves the position to point to 's'. */
int rn_int(int *pos, const char *str) {
    int opos = 0;
    char buf[20];
    memset(buf, 0, 20);
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
    tag_list *cur=list->tag_chain[**str];

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
        cur = cur->tag_chain[**str];
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
        if(islower(**str)) return TAG_TAG;
        else if(isupper(**str)) return TAG_FRAME;
        else if(**str == '-') return TAG_MARKER;
    } while(*((*str)++));
    
    (*str)--;
    return TAG_END;
}

int sd_stringparser_run(sd_stringparser *parser, unsigned long *ticks) {
    // Right, so
    // 1. Jump to the part of the string that "ticks" variable points to
    // 2. If the part has already been handled, just return.
    //    If not, parse the string part, call callbacks, etc.
    // 3. Return 0 for success, != 0 for somekind of error (use sd_error for returning string representation)

    if(parser->string) {
        int end_of_frame = 0;
        while(1) {
            const char *start = parser->string + *ticks;
            const char *str = parser->string + *ticks;

            int type = next_tag(&str);
            *ticks += (str-start);
            start = parser->string + *ticks;

            if(type == TAG_TAG) {
                // a tag
                tag_attribute attrib;
                if(rn_tag_attribute(parser->tag_list, &str, &attrib) == 0) {
                    // read the numeric param and call the callback function
                    // if a param is not present, 0 is assumed
                    int pos = 0;
                    int param = rn_int(&pos, str); 
                    str += pos;
                    if(attrib.callback) attrib.callback(parser, param);
                }
            } else if(type == TAG_FRAME) {
                // a frame
                int duration=0;
                char frame_letter=0;
                rn_frame(&str, &frame_letter, &duration);
            } else if(type == TAG_MARKER) {
                // an end of frame descriptor marker (a dash, '-')
                rn_descriptor_marker(&str);
                end_of_frame = 1;
            } else {
                // the end of stream
                return 1;
            }

            *ticks += (str-start);
            if(end_of_frame) return 0;
        }
    }

    assert(0 && "should never reach here!");
    return -1;
}
