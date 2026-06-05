#ifndef RESOURCE_FILES_H
#define RESOURCE_FILES_H

#include "utils/path.h"

path get_config_filename(void);
path get_log_filename(void);
path get_scores_filename(void);
path get_screenshot_filename(const char *timestamp);
path get_snapshot_rec_filename(const char *timestamp);
path get_save_directory(void);
bool scan_save_directory(list *results, const char *pattern);
path get_shader_filename(const char *shader_name);
path get_game_controller_db_filename(void);
path get_resource_filename(const char *resource_name);
bool scan_resource_path(list *results, const char *pattern);
path get_system_mod_directory(void);
path get_user_mod_directory(void);

#endif // RESOURCE_FILES_H
