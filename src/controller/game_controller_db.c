#include <SDL_gamecontroller.h>
#include <string.h>

#include "resources/pathmanager.h"
#include "utils/log.h"
#include "utils/str.h"

void joystick_load_external_mappings(void) {
    int loaded;
    str controller_db_path;
    const char *resource_path = pm_get_local_path(RESOURCE_PATH);
    str_from_format(&controller_db_path, "%sgamecontrollerdb.txt", resource_path);
    if((loaded = SDL_GameControllerAddMappingsFromFile(str_c(&controller_db_path))) > 0) {
        INFO("Loaded %d external game controller mappings from %s", loaded, str_c(&controller_db_path));
        INFO("We have %d known game controller mappings", SDL_GameControllerNumMappings());
    } else {
        INFO("No external game controller mappings file found from '%s' (%s); skipping ...", str_c(&controller_db_path),
             SDL_GetError());
    }
    str_free(&controller_db_path);
}
