#include <SDL_gamecontroller.h>
#include <string.h>

#include "resources/pathmanager.h"
#include "utils/log.h"
#include "utils/str.h"

void joystick_load_external_mappings() {
    str controller_db_path;
    const char *resource_path = pm_get_local_path(RESOURCE_PATH);
    str_from_format(&controller_db_path, "%sgamecontrollerdb.txt", resource_path);
    if(SDL_GameControllerAddMappingsFromFile(str_c(&controller_db_path)) > 0) {
        INFO("Loaded external game controller mappings from %s", str_c(&controller_db_path));
        INFO("We have %d known game controller mappings", SDL_GameControllerNumMappings());
    } else {
        INFO("No external game controller mappings file found from %s; skipping ...", str_c(&controller_db_path));
    }
    str_free(&controller_db_path);
}
