#ifndef XYSIZER_H
#define XYSIZER_H

#include "game/gui/component.h"
#include "video/surface.h"

typedef struct {
    int req_refresh;
    void *userdata;
} xysizer;

component *xysizer_create(void);
void xysizer_attach(component *sizer, component *c, int x, int y, int w, int h);
void xysizer_set_userdata(component *sizer, void *userdata);
void *xysizer_get_userdata(component *sizer);

#endif // XYSIZER_H
