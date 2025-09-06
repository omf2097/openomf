#ifndef RESOURCE_PATHS_H
#define RESOURCE_PATHS_H

#include "utils/path.h"

/**
 * Get config directory (users writeable for config files)
 */
path get_config_dir(void);

/**
 * Get state directory (users writeable for saved games, screenshots, etc.)
 */
path get_state_dir(void);

/**
 * Get resources directory (read-only game data)
 */
path get_resource_dir(void);

/**
 * Initialize resource paths. Should be called on startup.
 */
bool resource_path_init(void);

#endif // RESOURCE_PATHS_H
