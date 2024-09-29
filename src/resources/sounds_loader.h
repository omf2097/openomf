#ifndef SOUNDS_LOADER_H
#define SOUNDS_LOADER_H

int sounds_loader_init(void);
int sounds_loader_get(int id, char **buffer, int *len);
void sounds_loader_close(void);

#endif // SOUNDS_LOADER_H
