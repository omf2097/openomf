#ifndef _TEXTURELIST_H
#define _TEXTURELIST_H

#include "video/texture.h"

void texturelist_init();
void texturelist_close();
void texturelist_add(texture *tex);
void texturelist_remove(texture *tex);
void texturelist_revalidate_all();
unsigned int texturelist_get_bsize();

#endif // _TEXTURELIST_H