#ifndef OMF_RECT_H
#define OMF_RECT_H

// Rectangle in integer pixel coordinates.
typedef struct irect {
    int x;
    int y;
    int w;
    int h;
} irect;

static inline irect irect_create(int x, int y, int w, int h) {
    return (irect){x, y, w, h};
}

#endif // OMF_RECT_H
