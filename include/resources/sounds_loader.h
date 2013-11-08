#ifndef _SOUNDS_LOADER_H
#define _SOUNDS_LOADER_H

int sounds_loader_init();
int sounds_loader_get(int id, char **buffer, int *len);
void sounds_loader_close();

#endif // _SOUNDS_LOADER_H
