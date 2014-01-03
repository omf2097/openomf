#include "game/protos/object_specializer.h"
#include "game/protos/object.h"
#include "game/objects/har.h"
#include "utils/log.h"

int object_auto_specialize(object *obj, int specialization_id) {
    switch(specialization_id) {
        case SPECID_HAR:
            /*DEBUG("Object is specialized as a HAR");*/
            har_bootstrap(obj);
            return 0;
        case SPECID_PROJECTILE:
            DEBUG("Object is specialized as a projectile");
            return 0;
        default:
            DEBUG("Object is specialized as %d", specialization_id);
            return 1;
    }
}

