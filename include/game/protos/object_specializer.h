#ifndef OBJECT_SPECIALIZER_H
#define OBJECT_SPECIALIZER_H

typedef struct object_t object;

enum {
    SPECID_NONE = 0,
    SPECID_HAR = 1,
    SPECID_PROJECTILE,
    SPECID_HAZARD,
    SPECID_SCRAP
};

int object_auto_specialize(object *obj, int specialization_id);

#endif // OBJECT_SPECIALIZER_H
