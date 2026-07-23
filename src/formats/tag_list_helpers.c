#include "formats/tag_list_helpers.h"
#include "formats/tag_list.h"
#include <string.h>

bool script_tag_lookup(const char *buf, const int len, script_tag *tag) {
    for(int i = 0; i < tag_descriptor_count; i++) {
        const char *candidate = tag_descriptor_list[i].tag;
        if(candidate == NULL) {
            continue;
        }
        // Match only if the len chars are equal AND the whole candidate is checked.
        if(strncmp(candidate, buf, len) == 0 && candidate[len] == '\0') {
            if(tag != NULL) {
                *tag = (script_tag)i;
            }
            return true;
        }
    }
    return false;
}

const char *script_invalid_tag_name(char ch) {
    switch(ch) {
        case 'c':
            return "c";
        case 'o':
            return "o";
        case 'p':
            return "p";
        case 'z':
            return "z";
        default:
            return NULL;
    }
}

const char *script_get_frame_tag_name(const script_frame_tag *tag) {
    if(tag == NULL) {
        return NULL;
    }
    if(tag->key == TAG_INVALID) {
        return script_invalid_tag_name((char)tag->value);
    }
    return tag_descriptor_list[tag->key].tag;
}

const char *script_get_frame_tag_description(const script_frame_tag *tag) {
    if(tag == NULL || tag->key == TAG_INVALID) {
        return NULL;
    }
    return tag_descriptor_list[tag->key].description;
}
