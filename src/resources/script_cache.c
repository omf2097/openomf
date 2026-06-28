#include "resources/script_cache.h"

#include "formats/error.h"
#include "utils/allocator.h"
#include "utils/hashmap.h"
#include "utils/log.h"

typedef struct script_cache {
    hashmap scripts; ///< Maps an animation string to its script instance
} script_cache;

static script_cache state;

// Called by the hashmap as free callback.
static void free_value(void *value) {
    script *s = *(script **)value;
    script_free(s);
    omf_free(s);
}

void script_cache_init(void) {
    hashmap_create_cb(&state.scripts, free_value);
}

void script_cache_close(void) {
    hashmap_free(&state.scripts);
}

const script *script_cache_get(const char *str) {
    void *value;
    unsigned int value_len;
    if(hashmap_get_str(&state.scripts, str, &value, &value_len) == 0) {
        return *(script **)value;
    }

    script *s = omf_malloc(sizeof(script));
    script_create(s);
    int invalid_pos = 0;
    if(script_decode(s, str, &invalid_pos) != SD_SUCCESS) {
        crash_with_args("Failed to decode string '%s'; error at position %d.", str, invalid_pos);
    }
    hashmap_put_str(&state.scripts, str, &s, sizeof(s));
    log_debug("Cached script '%s'; cache size now %d.", str, hashmap_reserved(&state.scripts));
    return s;
}

void script_cache_clear(void) {
    hashmap_clear(&state.scripts);
}
