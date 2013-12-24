#ifndef _OBJECT_SPECIALIZER
#define _OBJECT_SPECIALIZER

typedef struct object_t object;

enum {
    SPECID_NONE = 0,
    SPECID_HAR = 0,
    SPECID_PROJECTILE,
    SPECID_SCRAP
};

int object_auto_specialize(object *obj, int specialization_id);

#endif // _OBJECT_SPECIALIZER
