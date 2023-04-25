#include <SDL_gamecontroller.h>
#include <string.h>

#include "controller/builtin_mappings.h"
#include "controller/game_controller_db.h"
#include "resources/pathmanager.h"
#include "utils/log.h"
#include "utils/str.h"

void joystick_load_builtin_mappings() {
    int loaded;
    SDL_RWops *rw = SDL_RWFromConstMem(builtin_controller_mappings, strlen(builtin_controller_mappings));
    if((loaded = SDL_GameControllerAddMappingsFromRW(rw, 1)) < 0) {
        PERROR("Failed to load builtin joystick mappings: %s", SDL_GetError());
    } else {
        INFO("Loaded %d built-in controller mappings", loaded);
        INFO("We currently have %d known game controller mappings", SDL_GameControllerNumMappings());
    }
}

void joystick_load_external_mappings() {
    int loaded;
    str controller_db_path;
    const char *resource_path = pm_get_local_path(RESOURCE_PATH);
    str_from_format(&controller_db_path, "%sgamecontrollerdb.txt", resource_path);
    if((loaded = SDL_GameControllerAddMappingsFromFile(str_c(&controller_db_path))) > 0) {
        INFO("Loaded %d external game controller mappings from %s", loaded, str_c(&controller_db_path));
        INFO("We have %d known game controller mappings", SDL_GameControllerNumMappings());
    } else {
        INFO("No external game controller mappings file found from %s; skipping ...", str_c(&controller_db_path));
    }
    str_free(&controller_db_path);
}
