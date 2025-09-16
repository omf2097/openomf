#include "resources/resource_files.h"

#include "resources/resource_paths.h"
#include "utils/log.h"

path get_config_filename(void) {
    path name = get_config_dir();
    path_append(&name, "openomf.conf");
    return name;
}

path get_log_filename(void) {
    path name = get_state_dir();
    path_append(&name, "openomf.log");
    return name;
}

path get_scores_filename(void) {
    path name = get_state_dir();
    path_append(&name, "SCORES.DAT");
    return name;
}

path get_screenshot_filename(const char *timestamp) {
    char name[64];
    snprintf(name, 64, "screenshot_%s.png", timestamp);

    path path = get_state_dir();
    path_append(&path, "screenshot");
    if(!path_exists(&path)) {
        path_mkdir(&path);
    }

    path_append(&path, name);
    return path;
}

path get_snapshot_rec_filename(const char *timestamp) {
    char name[64];
    snprintf(name, 64, "%s.rec", timestamp);

    path path = get_state_dir();
    path_append(&path, "rec");
    if(!path_exists(&path)) {
        path_mkdir(&path);
    }

    path_append(&path, name);
    return path;
}

path get_save_directory(void) {
    path name = get_state_dir();
    path_append(&name, "save");
    if(!path_exists(&name)) {
        path_mkdir(&name);
    }
    return name;
}

bool scan_save_directory(list *results, const char *pattern) {
    const path scan = get_save_directory();
    return path_glob(&scan, results, pattern);
}

path get_shader_filename(const char *shader_name) {
    path name = get_resource_dir();
    path_append(&name, "shaders", shader_name);
    return name;
}

path get_game_controller_db_filename(void) {
    path name = get_resource_dir();
    path_append(&name, "resources", "gamecontrollerdb.txt");
    return name;
}

path get_resource_filename(const char *resource_name) {
    path name = get_resource_dir();
    path_append(&name, "resources", resource_name);
    return name;
}

bool scan_resource_path(list *results, const char *pattern) {
    path scan = get_resource_dir();
    path_append(&scan, "resources");
    return path_glob(&scan, results, pattern);
}
