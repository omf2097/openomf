#include "shadowdive/stringparser.h"

#include <stdlib.h>
#include <string.h>

sd_stringparser* sd_stringparser_create() {
    sd_stringparser *parser = (sd_stringparser*)malloc(sizeof(sd_stringparser));
    sd_stringparser_reset(parser);
    return parser;
}

void sd_stringparser_delete(sd_stringparser *parser) {
    if(parser) {
        free(parser);
    }
}

void sd_stringparser_set_cbs(sd_stringparser *parser, sd_stringparser_cbs callbacks) {
    if(!parser) return;
    parser->cbs = callbacks;
}

void sd_stringparser_reset(sd_stringparser *parser) {
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
    return atoi(str);
}

int sd_stringparser_run(sd_stringparser *parser, unsigned long ticks) {
    // Right, so
    // 1. Jump to the part of the string that "ticks" var hits
    // 2. If the part has already been handled, just return.
    //    If not, parse the string part, call callbacks, etc.
    // 3. Return 0 for success, != 0 for somekind of error (use sd_error for returning string representation)
    return 0;
}
