#include "engine.h"

int main(int argc, char *argv[]) {
    // Init SDL2
    // Init logging
    // Init dumb_stdfiles()
    // ... other libs
    // Check if config file exists and read it
    
    if(engine_init()) {
        return 1;
    }
    
    engine_run();
    engine_close();
    
    
    // Deinit prev.

    return 0;
}