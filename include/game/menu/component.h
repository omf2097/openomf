#ifndef _COMPONENT_H
#define _COMPONENT_H

#include <SDL2/SDL.h>

typedef struct component_t component;

/*
* This is the basic component that you get by creating any textbutton, togglebutton, etc.
* The point is to abstract away rendering and event handling
*/
struct component_t {
    int x,y,w,h;
    // could use a bitmask for the different states
    int selected;
    int disabled;
    void *obj;
    void *userdata;
    
    void (*render)(component *c);
    int (*event)(component *c, SDL_Event *event);
    void (*layout)(component *c, int x, int y, int w, int h);
    void (*tick)(component *c);
    
    void (*click)(component *c, void *userdata);
    void (*toggle)(component *c, void *userdata, int option);
    void (*slide)(component *c, void *userdata, int pos);
};

void component_create(component *c);
void component_free(component *c);
void component_layout(component *c, int x, int y, int w, int h);
void component_click(component *c);

#endif // _COMPONENT_H
