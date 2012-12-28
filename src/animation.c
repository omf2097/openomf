#include "animation.h"
#include <stdlib.h>

sd_animation* sd_animation_create() {
    sd_animation *anim = (sd_animation*)malloc(sizeof(sd_animation));
    anim->overlay_table=NULL;
    return anim;
}

void sd_animation_delete(sd_animation *anim) {
    if (anim->overlay_table != NULL) {
        free(anim->overlay_table);
    }
    free(anim);
}

