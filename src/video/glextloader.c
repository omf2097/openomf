#include "video/glextloader.h"
#include "utils/log.h"
#include <GL/glew.h>

int glext_init() {
    int err = glewInit();
    if(err != GLEW_OK) {
        PERROR("Failed to initialize GLEW: %s.", glewGetErrorString(err));
        return 1;
    }
    DEBUG("GLEW Initialized.");
    return 0;
}