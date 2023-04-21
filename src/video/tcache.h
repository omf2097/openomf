#ifndef TCACHE_H
#define TCACHE_H

#include "video/screen_palette.h"
#include "video/surface.h"
#include <SDL.h>

void tcache_init(SDL_Renderer *renderer);
void tcache_reinit(SDL_Renderer *renderer);
void tcache_close();
void tcache_clear();
SDL_Texture *tcache_get(surface *sur, screen_palette *pal, char *remap_table, uint8_t pal_offset);
void tcache_tick(void);

#endif // TCACHE_H
