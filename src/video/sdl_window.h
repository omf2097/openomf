#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include <SDL.h>
#include <stdbool.h>

bool create_gl_context(SDL_GLContext **context, SDL_Window *window);
bool create_window(SDL_Window **window, int width, int height, bool fullscreen);
bool resize_window(SDL_Window *window, int width, int height, bool fullscreen);
bool set_vsync(bool enable);
void ortho2d(float *matrix, float left, float right, float bottom, float top);

#endif // GL_CONTEXT_H
