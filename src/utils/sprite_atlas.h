#ifndef SPRITE_ATLAS_H
#define SPRITE_ATLAS_H

#include <stdint.h>
#include "utils/rect.h"

typedef int32_t atlas_index;
typedef struct sprite_atlas sprite_atlas;

/**
 * Initialize a new sprite atlas.
 */
sprite_atlas *atlas_create(uint16_t width, uint16_t height);

/**
 * Free atlas and set the pointer to NULL. If argument NULL, this is a no-op.
 */
void atlas_free(sprite_atlas **atlas);

/**
 * Attempt to insert an area to the atlas
 * @param atlas Atlas to insert into
 * @param w Width of the inserted area
 * @param h Height of the inserted area
 * @param item Item coordinates and size, if successfully inserted. Can be NULL.
 * @return Atlas key if inserted, -1 if insert failed (no space).
 */
atlas_index atlas_insert(sprite_atlas *atlas, uint16_t w, uint16_t h, rect16 *item);

/**
 * Get item from atlas
 * @param atlas Atlas to get from
 * @param key Atlas key
 * @param item Item coordinates and size, if key was found.
 * @return true if key was found, false if not.
 */
bool atlas_get(const sprite_atlas *atlas, atlas_index key, rect16 *item);


#endif // SPRITE_ATLAS_H
