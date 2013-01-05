#include "shadowdive/stringparser.h"

#include <stdlib.h>

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

int sd_stringparser_run(sd_stringparser *parser, int ticks) {
    parser->ticks += ticks;
    return 0;
}
