#include <stdlib.h>
#include "video/tcache.h"
#include "utils/hashmap.h"
#include "utils/log.h"

#define CACHE_LIFETIME 25

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
} tcache;

static tcache *cache = NULL;

// Helper method for getting cache entry
void tcache_add_entry(tcache_entry_key *key, tcache_entry_value *val) {
    hashmap_put(&cache->entries, 
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

void tcache_init() {
    cache = malloc(sizeof(tcache));
    hashmap_create(&cache->entries, 6);
    cache->hits = 0;
    cache->misses = 0;
    DEBUG("Texture cache initialized.");
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
        }
    }
}

void tcache_close() {
    DEBUG("Texture cache:");
    DEBUG(" * Cache misses: %d", cache->misses);
    DEBUG(" * Cache hits: %d", cache->hits);
    tcache_clear();
    hashmap_free(&cache->entries);
    free(cache);
}

SDL_Texture* tcache_get(surface *sur, 
                        SDL_Renderer *renderer, 
                        screen_palette *pal, 
                        char *remap_table,
                        uint8_t pal_offset) {
    SDL_Texture *ret = NULL;

    // Form a key
    tcache_entry_key key;
    key.c_pal_offset = pal_offset;
    key.c_remap_table = remap_table;
    key.c_surface = sur;
    key.w = sur->w;
    key.h = sur->h;

    // Attempt to find appropriate surface
    tcache_entry_value *val = tcache_get_entry(&key);
    if(val == NULL) {
        // Create a new entry
        tcache_entry_value new_entry;
        new_entry.age = 0;
        new_entry.pal_version = pal->version;
        new_entry.tex = SDL_CreateTexture(renderer, 
                                          SDL_PIXELFORMAT_ABGR8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          sur->w,
                                          sur->h);

        // Render surface to texture
        void *pixels;
        int pitch;
        int s = SDL_LockTexture(new_entry.tex, NULL, &pixels, &pitch);
        if(s == 0) {
            surface_to_rgba(sur, pixels, pal, remap_table, pal_offset);
            SDL_UnlockTexture(new_entry.tex);
        } else {
            PERROR("Unable to lock texture for writing!");
        }
        
        // Return value
        ret = new_entry.tex;

        // Add new entry to cache
        tcache_add_entry(&key, &new_entry);

        cache->misses++;
    } else {
        // Palette used is old, we need to update the texture
        if(val->pal_version != pal->version) {
            // Update texture contents with updated surface
            void *pixels;
            int pitch;
            int s = SDL_LockTexture(val->tex, NULL, &pixels, &pitch);
            if(s == 0) {
                surface_to_rgba(sur, pixels, pal, remap_table, pal_offset);
                SDL_UnlockTexture(val->tex);
            } else {
                PERROR("Unable to lock texture for writing!");
            }

            //  Set correct age and latest palette version
            val->age = 0;
            val->pal_version = pal->version;

            // Keep statistics up-to-date :)
            cache->misses++;
        } else {
            cache->hits++;
        }

        // Return cached texture
        ret = val->tex;
    }

    return ret;
}