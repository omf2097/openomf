#ifndef REMAPS_H
#define REMAPS_H

#include <epoxy/gl.h>

typedef struct remaps remaps;

remaps *remaps_create();
void remaps_update(remaps *remaps, const unsigned char *data);
GLuint remaps_get_block_id(const remaps *remaps);
void remaps_free(remaps **maps);

#endif // REMAPS_H
