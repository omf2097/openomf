#ifndef REMAPS_H
#define REMAPS_H

#include "video/vga_remap.h"
#include <epoxy/gl.h>

typedef struct remaps remaps;

remaps *remaps_create(GLuint unit_id);
void remaps_update(const remaps *remaps, vga_remap_tables *data);
void remaps_free(remaps **maps);

#endif // REMAPS_H
