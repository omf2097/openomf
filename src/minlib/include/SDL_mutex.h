#ifndef SDL_MUTEX_H
#define SDL_MUTEX_H

typedef struct SDL_mutex SDL_mutex;

SDL_mutex *SDL_CreateMutex(void);
void SDL_DestroyMutex(SDL_mutex *mutex);
int SDL_LockMutex(SDL_mutex *mutex);
int SDL_UnlockMutex(SDL_mutex *mutex);

#endif /* SDL_MUTEX_H */
