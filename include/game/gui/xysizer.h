#ifndef _XYSIZER_H
#define _XYSIZER_H

#include "video/surface.h"
#include "game/gui/component.h"

typedef struct  {
    int req_refresh;
} xysizer;

component* xysizer_create();
void xysizer_attach(component *menu, component *c, int x, int y, int w, int h);

#endif // _XYSIZER_H
