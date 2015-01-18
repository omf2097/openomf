#include "game/scenes/mechlab/lab_newplayer.h"
#include "game/gui/xysizer.h"

component* lab_newplayer_create(scene *s) {

    // This function should contain component initialization for creating the new player creation stuff

    return xysizer_create();
}