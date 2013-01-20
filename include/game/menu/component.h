#ifndef _COMPONENT_H
#define _COMPONENT_H

typedef struct component_t component;

/*
* This is the basic comonent that you get by creating any textbutton, togglebutton, etc.
* The point is to abstract away rendering and event handling
*/
struct component_t {
    void *obj;
    void (*render)(void *obj);
    void (*click)(void *obj);
    void (*select_left)(void *obj);
    void (*select_right)(void *obj);
};

#endif // _COMPONENT_H