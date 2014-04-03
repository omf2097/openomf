#ifndef _TCACHE_H
#define _TCACHE_H

#include <SDL2/SDL.h>
#include "video/surface.h"
#include "video/screen_palette.h"
#include "plugins/scaler_plugin.h"

void tcache_init(SDL_Renderer *renderer, int scale_factor, scaler_plugin *scaler);
void tcache_reinit(SDL_Renderer *renderer, int scale_factor, scaler_plugin *scaler);
void tcache_close();
void tcache_clear();
SDL_Texture* tcache_get(surface *sur, 
                        screen_palette *pal, 
                        char *remap_table,
                        uint8_t pal_offset);
void tcache_tick();

#endif // _TCACHE_H
