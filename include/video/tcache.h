#ifndef _TCACHE_H
#define _TCACHE_H

#include <SDL2/SDL.h>
#include "video/surface.h"
#include "video/screen_palette.h"

void tcache_init(int scale_factor);
void tcache_close();
void tcache_clear();
SDL_Texture* tcache_get(surface *sur, 
                        SDL_Renderer *renderer, 
                        screen_palette *pal, 
                        char *remap_table,
                        uint8_t pal_offset);
void tcache_tick();

#endif // _TCACHE_H
