#include "formats/error.h"
#include "formats/taglist.h"
#include <stdlib.h>
#include <string.h>

int sd_tag_info(const char *search_tag, int *req_param, const char **tag, const char **desc) {
    for(int i = 0; i < sd_taglist_size; i++) {
        if(strcmp(search_tag, sd_taglist[i].tag) == 0) {
            if(req_param != NULL)
                *req_param = sd_taglist[i].has_param;
            if(tag != NULL)
                *tag = sd_taglist[i].tag;
            if(desc != NULL)
                *desc = sd_taglist[i].description;
            return SD_SUCCESS;
        }
    }
    return SD_INVALID_INPUT;
}
