#include "game/protos/object_specializer.h"
#include "game/protos/object.h"
#include "game/objects/har.h"

int object_auto_specialize(object *obj, int specialization_id) {
    switch(specialization_id) {
        case SPECID_HAR:
            har_bootstrap(obj);
            return 0;
        case SPECID_PROJECTILE:
            return 0;
        default:
            return 1;
    }
}

