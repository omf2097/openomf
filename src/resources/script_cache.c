#include "resources/script_cache.h"

#include "formats/error.h"
#include "utils/allocator.h"
#include "utils/log.h"

// Called by the hashmap as free callback.
static void free_value(void *value) {
    script *s = *(script **)value;
    script_free(s);
    omf_free(s);
}

void script_cache_init(script_cache *cache) {
    hashmap_create_cb(&cache->scripts, free_value);
}

void script_cache_close(script_cache *cache) {
    hashmap_free(&cache->scripts);
}

const script *script_cache_get(script_cache *cache, const char *str) {
    void *value;
    unsigned int value_len;
    if(hashmap_get_str(&cache->scripts, str, &value, &value_len) == 0) {
        return *(script **)value;
    }

    script *s = omf_malloc(sizeof(script));
    script_create(s);
    int invalid_pos = 0;
    if(script_decode(s, str, &invalid_pos) != SD_SUCCESS) {
        crash_with_args("Failed to decode string '%s'; error at position %d.", str, invalid_pos);
    }
    hashmap_put_str(&cache->scripts, str, &s, sizeof(s));
    log_debug("Cached script '%s'; cache size now %u.", str, hashmap_reserved(&cache->scripts));
    return s;
}

void script_cache_clear(script_cache *cache) {
    hashmap_clear(&cache->scripts);
}
