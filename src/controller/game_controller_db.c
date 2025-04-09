#include <SDL_gamecontroller.h>

#include "resources/resource_files.h"
#include "utils/log.h"
#include "utils/path.h"
#include "utils/str.h"

void joystick_load_external_mappings(void) {
    int loaded;
    path db_filename = get_game_controller_db_filename();
    if((loaded = SDL_GameControllerAddMappingsFromFile(path_c(&db_filename))) > 0) {
        log_info("Loaded %d external game controller mappings from %s", loaded, path_c(&db_filename));
        log_info("We have %d known game controller mappings", SDL_GameControllerNumMappings());
    } else {
        log_info("No external game controller mappings file found from '%s' (%s); skipping ...", path_c(&db_filename),
                 SDL_GetError());
    }
}
