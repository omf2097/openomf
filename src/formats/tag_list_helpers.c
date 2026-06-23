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
