#include <stdlib.h>
#include "video/tcache.h"
#include "utils/hashmap.h"
#include "utils/log.h"

#define CACHE_LIFETIME 300

typedef struct tcache_entry_key_t {
    surface *c_surface;
    char *c_remap_table;
    uint16_t w,h;
    uint8_t c_pal_offset;
} tcache_entry_key;

typedef struct tcache_entry_value_t {
    SDL_Texture *tex;
    unsigned int age;
    unsigned int pal_version;
} tcache_entry_value;

typedef struct tcache_t {
    hashmap entries;
    unsigned int hits;
    unsigned int misses;
    unsigned int old_frees;
    uint8_t scale_factor;
    scaler_plugin *scaler;
    SDL_Renderer *renderer;
} tcache;

static tcache *cache = NULL;

// Helper method for getting cache entry
tcache_entry_value* tcache_add_entry(tcache_entry_key *key, tcache_entry_value *val) {
    return hashmap_put(&cache->entries,
                      (void*)key, sizeof(tcache_entry_key),
                      (void*)val, sizeof(tcache_entry_value));
}

// Helper method for setting cache entry
tcache_entry_value* tcache_get_entry(tcache_entry_key *key) {
    tcache_entry_value *val = NULL;
    unsigned int tmp_size;
    hashmap_get(&cache->entries,
                (void*)key, sizeof(tcache_entry_key),
                (void**)&val, &tmp_size);
    return val;
}

void tcache_init(SDL_Renderer *renderer, int scale_factor, scaler_plugin *scaler) {
    cache = malloc(sizeof(tcache));
    hashmap_create(&cache->entries, 6);
    cache->renderer = renderer;
    cache->scaler = scaler;
    cache->scale_factor = scale_factor;
    cache->hits = 0;
    cache->old_frees = 0;
    cache->misses = 0;
    DEBUG("Texture cache initialized.");
}

void tcache_reinit(SDL_Renderer *renderer, int scale_factor, scaler_plugin *scaler) {
    cache->renderer = renderer;
    cache->scaler = scaler;
    cache->scale_factor = scale_factor;
    tcache_clear();
}

void tcache_clear() {
    iterator it;
    hashmap_iter_begin(&cache->entries, &it);
    hashmap_pair *pair;
    while((pair = iter_next(&it)) != NULL) {
        tcache_entry_value *entry = pair->val;
        SDL_DestroyTexture(entry->tex);
    }
    hashmap_clear(&cache->entries);
}

void tcache_tick() {
    iterator it;
    hashmap_iter_begin(&cache->entries, &it);
    hashmap_pair *pair;
    while((pair = iter_next(&it)) != NULL) {
        tcache_entry_value *entry = pair->val;
        entry->age++;
        if(entry->age > CACHE_LIFETIME) {
            SDL_DestroyTexture(entry->tex);
            hashmap_delete(&cache->entries, &it);
            cache->old_frees++;
        }
    }
}

void tcache_close() {
    DEBUG("Texture cache:");
    DEBUG(" * Misses:    %d", cache->misses);
    DEBUG(" * Hits:      %d", cache->hits);
    DEBUG(" * Old frees: %d", cache->old_frees);
    tcache_clear();
    hashmap_free(&cache->entries);
    free(cache);
}

SDL_Texture* tcache_get(surface *sur,
                        screen_palette *pal,
                        char *remap_table,
                        uint8_t pal_offset) {
    if(sur == NULL) {
        DEBUG("Invalid surface requested from tcache: surface is NULL.");
        return NULL;
    }

    if(sur->w == 0 || sur->h == 0 || sur->data == NULL) {
        DEBUG("Invalid surface requested from tcache: w,h = %d,%d data = %p", sur->w, sur->h, sur->data);
        return NULL;
    }

    // Form a key
    tcache_entry_key key;
    memset(&key, 0, sizeof(tcache_entry_key));
    key.c_pal_offset = (sur->type == SURFACE_TYPE_RGBA) ? 0 : pal_offset;
    key.c_remap_table = (sur->type == SURFACE_TYPE_RGBA) ? 0 : remap_table;
    key.c_surface = sur;
    key.w = sur->w;
    key.h = sur->h;

    // Attempt to find appropriate surface
    // If surface is cacheable and hasn't changed, just return here.
    tcache_entry_value *val = tcache_get_entry(&key);
    if(val != NULL && (val->pal_version == pal->version || sur->type == SURFACE_TYPE_RGBA) && !sur->force_refresh) {
        val->age = 0;
        cache->hits++;
        return val->tex;
    }

    // Reset refresh flag here
    sur->force_refresh = 0;

    // If there was no fitting surface tex in the cache at all,
    // then we need to create one
    if(val == NULL) {
        tcache_entry_value new_entry;
        new_entry.age = 0;
        new_entry.pal_version = pal->version;
        new_entry.tex = SDL_CreateTexture(cache->renderer,
                                          SDL_PIXELFORMAT_ABGR8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          sur->w * cache->scale_factor,
                                          sur->h * cache->scale_factor);
        SDL_SetTextureBlendMode(new_entry.tex, SDL_BLENDMODE_BLEND);
        val = tcache_add_entry(&key, &new_entry);
    }

    // We have a texture either from the cache, or we just created one.
    // Either one, it needs to be updated. Let's do it now.
    // Also, scale surface if necessary
    if(cache->scale_factor > 1) {
        char *raw = malloc(sur->w * sur->h * 4);
        surface scaled;
        surface_create(&scaled,
                       SURFACE_TYPE_RGBA,
                       sur->w * cache->scale_factor,
                       sur->h * cache->scale_factor);

        surface_to_rgba(sur, raw, pal, remap_table, pal_offset);
        scaler_scale(cache->scaler, raw, scaled.data, sur->w, sur->h, cache->scale_factor);
        surface_to_texture(&scaled, val->tex, pal, remap_table, pal_offset);
        surface_free(&scaled);
        free(raw);
    } else {
        surface_to_texture(sur, val->tex, pal, remap_table, pal_offset);
    }

    // Set correct age and palette version
    val->age = 0;
    val->pal_version = pal->version;

    // Do some statistics stuff
    cache->misses++;
    return val->tex;
}
