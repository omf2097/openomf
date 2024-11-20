#ifndef SOUNDS_LOADER_H
#define SOUNDS_LOADER_H

#include <stdbool.h>

bool sounds_loader_init(void);
bool sounds_loader_get(int id, char **buffer, int *len);
void sounds_loader_close(void);

#endif // SOUNDS_LOADER_H
