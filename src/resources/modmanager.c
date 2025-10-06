#include "resources/modmanager.h"

#include "formats/error.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "vendored/zip/zip.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef OPUSFILE_FOUND
#include <opusfile.h>
#endif

#include <confuse.h>

hashmap mod_resources;

// types of mod asset storage, buffer is for things like ini files or opus files
typedef enum
{
    MOD_VGA_IMAGE,
    MOD_SPRITE,
    MOD_BUFFER
} asset_type;

// Manifest structure to track mod information
typedef struct {
    char *name;     /* Unique name of the mod */
    char *mod_api;  /* API version, should be "1.0" for now */
    char *version;  /* Semver version string */
    int load_order; /* Load order (lower numbers load first) */
    path *filepath; /* Path to the mod file */
} mod_manifest;

// Free a manifest structure's internal fields (used as hashmap callback)
static void free_manifest_fields(void *data) {
    mod_manifest *manifest = (mod_manifest *)data;
    if(manifest) {
        omf_free(manifest->name);
        omf_free(manifest->mod_api);
        omf_free(manifest->version);
        // Don't free filepath as it's owned by mod_list
    }
}

// union type to cover all the mod asset types
typedef struct {
    asset_type type;
    size_t size;
    union {
        unsigned char *buf;
        sd_vga_image img;
        sd_sprite spr;
    };
} mod_asset;

int mod_find(list *mod_list) {
    size_t size = 0;
    path scan = get_system_mod_directory();
    if(!path_glob(&scan, mod_list, "*.zip")) {
        log_warn("Failed to scan system mods directory!");
    } else {
        log_info("Found %d system mods.", list_size(mod_list));
    }
    size = list_size(mod_list);
    scan = get_user_mod_directory();
    if(!path_glob(&scan, mod_list, "*.zip")) {
        log_warn("Failed to scan user mods directory!");
    } else {
        log_info("Found %d user mods.", list_size(mod_list) - size);
    }
    size = list_size(mod_list);

    return size;
}

static int compare_versions(const char *a, const char *b) {
    long a_parts[3] = {0, 0, 0};
    long b_parts[3] = {0, 0, 0};

    // Parse version strings, scanf will not populate unmatched fields
    sscanf(a, "%ld.%ld.%ld", &a_parts[0], &a_parts[1], &a_parts[2]);
    sscanf(b, "%ld.%ld.%ld", &b_parts[0], &b_parts[1], &b_parts[2]);

    // Compare components
    for(int i = 0; i < 3; i++) {
        if(a_parts[i] < b_parts[i])
            return -1;
        if(a_parts[i] > b_parts[i])
            return 1;
    }

    return 0;
}

// Parse manifest.ini content and determine if this mod should be used
// Adds valid mods to the mod_registry
static bool parse_manifest_and_add_to_modlist(const char *buf, path *mod_path, hashmap *mod_registry) {
    assert(buf != NULL);
    assert(mod_path != NULL);
    assert(mod_registry != NULL);

    cfg_opt_t manifest_opts[] = {CFG_STR("name", "", CFGF_NONE), CFG_STR("mod_api", "", CFGF_NONE),
                                 CFG_STR("version", "", CFGF_NONE), CFG_INT("load_order", 0, CFGF_NONE), CFG_END()};

    cfg_t *cfg = cfg_init(manifest_opts, CFGF_IGNORE_UNKNOWN);

    if(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR) {
        log_error("Failed to parse manifest.ini");
        cfg_free(cfg);
        return false;
    }

    // Get required fields
    char *name = cfg_getstr(cfg, "name");
    char *mod_api = cfg_getstr(cfg, "mod_api");
    char *version = cfg_getstr(cfg, "version");
    int load_order = cfg_getint(cfg, "load_order");

    // Validate required fields individually
    if(strlen(name) == 0) {
        log_error("Manifest is missing required 'name' field");
        cfg_free(cfg);
        return false;
    }

    if(strlen(mod_api) == 0) {
        log_error("Manifest is missing required 'mod_api' field");
        cfg_free(cfg);
        return false;
    }

    if(strlen(version) == 0) {
        log_error("Manifest is missing required 'version' field");
        cfg_free(cfg);
        return false;
    }

    // Check if mod API version is supported
    // For now, we only support 1.0
    if(strcmp(mod_api, "1.0") != 0) {
        log_warn("Mod %s has unsupported API version %s, ignoring", name, mod_api);
        cfg_free(cfg);
        return false;
    }

    // Check for existing mod with same name
    mod_manifest *existing;
    unsigned int len;
    if(hashmap_get_str(mod_registry, name, (void **)&existing, &len) == 0) {
        // Found a mod with the same name, compare versions
        int compare = compare_versions(version, existing->version);
        if( compare > 0) {
            // New mod has a higher version, update the existing entry
            log_info("Replacing mod %s v%s with v%s", name, existing->version, version);

            // Update the existing entry with new information
            existing->load_order = load_order;

            cfg_free(cfg);
            return true;
        } else if (compare == 0) {
            log_warn("Mod %s v%s already exists in %s, ignoring %s!", name, version, path_c(existing->path), path_c(mod_path));
            cfg_free(cfg);
            return false;
        } else {
            // Existing mod has higher version
            log_info("Ignoring mod %s v%s in favor of existing v%s", name, version, existing->version);
            cfg_free(cfg);
            return false;
        }
    }

    // Create manifest entry
    mod_manifest manifest;
    memset(&manifest, 0, sizeof(mod_manifest));
    manifest.name = strdup(name);
    manifest.mod_api = strdup(mod_api);
    manifest.version = strdup(version);
    manifest.load_order = load_order;
    manifest.filepath = omf_calloc(1, sizeof(path));
    path_from_c(manifest.filepath, path_c(mod_path));

    // Add to registry for conflict resolution
    hashmap_put_str(mod_registry, name, &manifest, sizeof(mod_manifest));

    cfg_free(cfg);
    return true;
}

// Build registry of mods from directory list
static void build_mod_registry(list *dir_list, hashmap *mod_registry) {
    iterator it;
    path *p;

    // Parse all manifests and create a list of valid mods
    list_iter_begin(dir_list, &it);
    foreach(it, p) {
        struct zip_t *zip = zip_open(path_c(p), 0, 'r');
        if(zip_entry_opencasesensitive(zip, "manifest.ini") == 0) {
            // Read the manifest.ini content
            unsigned long long entry_size = zip_entry_uncomp_size(zip);
            void *entry_buf = omf_calloc(entry_size + 1, 1); // +1 for null terminator

            if(zip_entry_noallocread(zip, entry_buf, entry_size) < 0) {
                log_warn("Failed to read manifest.ini from %s", path_c(p));
                omf_free(entry_buf);
                zip_entry_close(zip);
                zip_close(zip);
                continue;
            }

            // Ensure null termination for the buffer
            ((char *)entry_buf)[entry_size] = '\0';

            // Parse the manifest and add/replace in registry
            parse_manifest_and_add_to_modlist(entry_buf, p, mod_registry);
            omf_free(entry_buf);
            zip_entry_close(zip);
        } else {
            log_warn("Mod %s is lacking a manifest.ini, ignoring", path_c(p));
        }
        zip_close(zip);
    }
}

// Sort mods from registry into a list ordered by load_order
static void mod_sort(hashmap *mod_registry, list *sorted_list) {

    // Extract mods from registry and sort them by load_order
    iterator iter;
    hashmap_iter_begin(mod_registry, &iter);
    hashmap_pair *pair;

    // First count the number of manifests by iterating
    int manifest_count = 0;
    foreach(iter, pair) {
        manifest_count++;
    }

    // Reset iterator to beginning
    hashmap_iter_begin(mod_registry, &iter);

    // Create array of the right size for sorting
    mod_manifest **manifests = NULL;
    if(manifest_count > 0) {
        manifests = omf_calloc(manifest_count, sizeof(mod_manifest *));
        int i = 0;
        foreach(iter, pair) {
            mod_manifest *manifest = (mod_manifest *)pair->value;
            manifests[i++] = manifest;
        }
    }

    log_info("loaded %d valid manifests", manifest_count);

    // Simple bubble sort by load_order - only if we have mods to sort
    if(manifest_count > 1 && manifests != NULL) {
        for(int j = 0; j < manifest_count; j++) {
            for(int k = 0; k < manifest_count - j - 1; k++) {
                if(manifests[k]->load_order > manifests[k + 1]->load_order) {
                    mod_manifest *temp = manifests[k];
                    manifests[k] = manifests[k + 1];
                    manifests[k + 1] = temp;
                }
            }
        }
    }

    // Add sorted paths to new list
    for(int i = 0; i < manifest_count && manifests != NULL; i++) {
        list_append(sorted_list, manifests[i]->filepath, sizeof(path));
        log_info("Adding mod %s v%s (load order: %d)", manifests[i]->name, manifests[i]->version,
                 manifests[i]->load_order);
    }

    // Free only the array we created for sorting
    // Don't free the manifests themselves or their filepaths
    // as those are owned by the mod_registry and used by mod_list
    if(manifests != NULL) {
        omf_free(manifests);
    }
}

bool modmanager_init(void) {

    hashmap_create(&mod_resources);
    list dir_list;
    list_create(&dir_list);
    mod_find(&dir_list);

    list mod_list;
    list_create(&mod_list);

    // Create and populate mod registry with valid mods from the directories
    hashmap mod_registry;
    hashmap_create_cb(&mod_registry, free_manifest_fields);
    build_mod_registry(&dir_list, &mod_registry);

    // Sort the mods by load_order and create a properly ordered list
    mod_sort(&mod_registry, &mod_list);

    // Free registry and its contents - the free callback will handle the internal fields
    hashmap_free(&mod_registry);

    iterator it;
    path *p;
    list_iter_begin(&mod_list, &it);
    foreach(it, p) {
        struct zip_t *zip = zip_open(path_c(p), 0, 'r');
        ssize_t entries = zip_entries_total(zip);
        if(entries > 0) {
            log_info("mod %s has %d files", path_c(p), entries);
            for(size_t i = 0; i < (size_t)entries; i++) {
                if(zip_entry_openbyindex(zip, i) == 0 && zip_entry_isdir(zip) == 0) {
                    log_info("mod contains %s", zip_entry_name(zip));
                    str filename;
                    str_create(&filename);
                    str_from_c(&filename, zip_entry_name(zip));
                    str_tolower(&filename);

                    if(strcmp("manifest.ini", str_c(&filename)) == 0) {
                        // this was loaded before
                        str_free(&filename);
                        zip_entry_close(zip);
                        continue;
                    }

                    unsigned long long entry_size = zip_entry_uncomp_size(zip);
                    void *entry_buf = omf_calloc(entry_size, 1);
                    if(zip_entry_noallocread(zip, entry_buf, entry_size) < 0) {
                        log_warn("failed to load %s into memory", zip_entry_name(zip));
                    }

                    path path;
                    str fn, ext;
                    path_from_str(&path, &filename);

                    path_filename(&path, &fn);
                    path_ext(&path, &ext);

                    log_info("path %s has filename %s and extension %s", str_c(&filename), str_c(&fn), str_c(&ext));

                    if(strcmp("background.png", str_c(&fn)) == 0) {
                        // parse as background image
                        mod_asset *buf = omf_calloc(1, sizeof(mod_asset));
                        buf->type = MOD_VGA_IMAGE;

                        if(sd_vga_image_from_png_in_memory(&buf->img, entry_buf, entry_size, false) == SD_SUCCESS) {

                            log_info("got vga image %dx%d", buf->img.w, buf->img.h);

                            hashmap_put_str(&mod_resources, str_c(&filename), buf, sizeof(mod_asset));
                        } else {
                            log_warn("failed to load background image %s", str_c(&filename));
                        }
                    } else if(strcmp(".png", str_c(&ext)) == 0) {
                        // parse as sprite
                        sd_vga_image img;
                        if(sd_vga_image_from_png_in_memory(&img, entry_buf, entry_size, true) == SD_SUCCESS) {
                            mod_asset *buf = omf_calloc(1, sizeof(mod_asset));
                            buf->type = MOD_SPRITE;
                            if(sd_sprite_vga_encode(&buf->spr, &img) == SD_SUCCESS) {
                                hashmap_put_str(&mod_resources, str_c(&filename), buf, sizeof(mod_asset));
                            } else {
                                log_warn("failed to load sprite %s", str_c(&filename));
                            }
                        }
                    } else if(strcmp(".ini", str_c(&ext)) == 0) {
                        list *l;
                        unsigned int len;
                        char *ini_buf = omf_calloc(1, entry_size + 1);
                        mod_asset *buf = omf_calloc(1, sizeof(mod_asset));
                        buf->size = entry_size + 1;
                        buf->type = MOD_BUFFER;

                        // XXX the file buffer is NOT null terminated
                        strncpy(ini_buf, entry_buf, entry_size);

                        buf->buf = (unsigned char *)ini_buf;

                        omf_free(entry_buf);

                        if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
                            list_append(l, buf, sizeof(mod_asset));
                        } else {
                            l = omf_calloc(1, sizeof(list));
                            list_create(l);
                            list_append(l, buf, sizeof(mod_asset));
                            hashmap_put_str(&mod_resources, str_c(&filename), l, sizeof(list));
                        }
#ifdef OPUSFILE_FOUND
                    } else if(strcmp(".ogg", str_c(&ext)) == 0) {
                        if(op_test(NULL, entry_buf, entry_size) == 0) {
                            log_info("got OPUS file %s", str_c(&filename));
                            list *l;
                            unsigned int len;
                            mod_asset *buf = omf_calloc(1, sizeof(mod_asset));
                            buf->size = entry_size;
                            buf->type = MOD_BUFFER;
                            buf->buf = entry_buf;
                            if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
                                list_append(l, buf, sizeof(mod_asset));
                            } else {
                                l = omf_calloc(1, sizeof(list));
                                list_create(l);
                                list_append(l, buf, sizeof(mod_asset));
                                hashmap_put_str(&mod_resources, str_c(&filename), l, sizeof(list));
                            }
                        } else {
                            log_warn("Failed to parse OPUS file %s", str_c(&filename));
                        }
#endif
                    }
                    str_free(&filename);
                }
                zip_entry_close(zip);
            }
        } else {
            log_warn("mod %s has no contents", path_c(p));
        }
        zip_close(zip);
    }

    list_free(&dir_list);
    return true;
}

bool modmanager_get_bk_background(str *name, sd_vga_image **img) {
    str filename;
    str_from_format(&filename, "scenes/%s/background.png", str_c(name));

    str_tolower(&filename);

    unsigned int len;
    bool found = false;
    mod_asset *obuf;
    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&obuf, &len)) {
        assert(obuf->type == MOD_VGA_IMAGE);
        *img = &obuf->img;
        log_info("got vga image %dx%d with size %d", (*img)->w, (*img)->h, len);
        found = true;
    }
    str_free(&filename);
    return found;
}

bool modmanager_get_sprite(animation_source source, str *name, int animation, int frame, sd_sprite **spr) {
    str filename;

    mod_asset *obuf;
    switch(source) {
        case AF_ANIMATION:
            str_from_format(&filename, "fighters/%s/%d/%d.png", str_c(name), animation, frame);
            break;
        case BK_ANIMATION:
            str_from_format(&filename, "scenes/%s/%d/%d.png", str_c(name), animation, frame);
            break;
        default:
            return false;
    }

    str_tolower(&filename);

    unsigned int len;
    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&obuf, &len)) {
        assert(obuf->type == MOD_SPRITE);
        *spr = &obuf->spr;
        log_info("got sprite %dx%d with size %d", (*spr)->width, (*spr)->height, len);
        str_free(&filename);
        return true;
    }

    str_free(&filename);

    // Common replacements should replace default assets, but not modded ones
    if(source == BK_ANIMATION && ((animation >= 6 && animation <= 11) || (animation >= 24 && animation <= 27))) {
        // TODO make sure this is an arena
        // For arenas, check for 'common' for animations 6 (round), 7 (number), 8 (you lose), 9 (you win), 10 (fight),
        // 11 (ready), 24 (dust 1), 25 (dust 2), 26 (dust 3), 27 (match counters)

        str_from_format(&filename, "scenes/common/%d/%d.png", animation, frame);
    } else if(source == AF_ANIMATION && (animation == 7 || animation == 8 || (animation >= 12 && animation <= 14) ||
                                         (animation >= 55 && animation <= 57))) {
        // For fighters, check for 'common' for animations 7 (burning oil/stun), 8 (blocking scrape), 12 (scrap), 13
        // (bolt), 14 (screw), 55 (blast), 56 (blast 2), 57 (blast 3)

        str_from_format(&filename, "fighters/common/%d/%d.png", animation, frame);
    } else {
        return false;
    }

    bool found = false;
    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&obuf, &len)) {
        assert(obuf->type == MOD_SPRITE);
        *spr = &obuf->spr;
        log_info("got sprite %dx%d with size %d", (*spr)->width, (*spr)->height, len);
        found = true;
    }

    str_free(&filename);

    return found;
}

unsigned int modmanager_count_music(str *name) {
    str filename;

    str_from_format(&filename, "audio/music/%s.ogg", str_c(name));
    str_tolower(&filename);

    list *l;
    unsigned int len = 0;

    int result = 0;

    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
        result = list_size(l);
    }

    str_free(&filename);

    return result;
}

bool modmanager_get_music(str *name, unsigned int index, unsigned char **buf, size_t *buflen) {
    str filename;

    str_from_format(&filename, "audio/music/%s.ogg", str_c(name));
    str_tolower(&filename);

    list *l;
    unsigned int len = 0;

    bool found = false;

    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
        unsigned int count = list_size(l);
        log_info("found %d music files for %s", count, name);
        if(index >= count) {
            log_warn("requested index %s into list of %d members", index, count);
            return false;
        }
        mod_asset *obuf = list_get(l, index);
        assert(obuf != NULL);
        *buf = obuf->buf;
        *buflen = obuf->size;
        found = true;
    }

    str_free(&filename);

    return found;
}

bool modmanager_parse_af_move_mod(const char *buf, af_move *current_move) {
    if(!buf || !current_move)
        return false;

    cfg_opt_t af_opts[] = {
        CFG_INT("pos_constraints", current_move->pos_constraints, CFGF_NONE),
        CFG_INT("next_move", current_move->next_move, CFGF_NONE),
        CFG_INT("successor_id", current_move->successor_id, CFGF_NONE),
        CFG_INT("category", current_move->category, CFGF_NONE), CFG_INT("points", current_move->points, CFGF_NONE),
        CFG_INT("block_damage", current_move->block_damage, CFGF_NONE),
        CFG_INT("block_stun", current_move->block_stun, CFGF_NONE),
        CFG_INT("throw_duration", current_move->throw_duration, CFGF_NONE),
        CFG_INT("extra_string_selector", current_move->extra_string_selector, CFGF_NONE),
        CFG_FLOAT("damage", current_move->damage, CFGF_NONE), CFG_FLOAT("stun", current_move->stun, CFGF_NONE),
        CFG_STR("move_string", str_c(&current_move->move_string), CFGF_NONE),
        CFG_STR("footer_string", str_c(&current_move->footer_string), CFGF_NONE),

        // Animation-specific options
        CFG_INT("start_x", current_move->ani.start_pos.x, CFGF_NONE),
        CFG_INT("start_y", current_move->ani.start_pos.y, CFGF_NONE),
        CFG_STR("animation_string", str_c(&current_move->ani.animation_string), CFGF_NONE), CFG_END()};

    cfg_t *cfg = cfg_init(af_opts, CFGF_NONE);

    if(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR) {
        cfg_free(cfg);
        return false;
    }

    // Update integer fields only if they changed
    if(current_move->pos_constraints != cfg_getint(cfg, "pos_constraints")) {
        log_info("setting pos_constraints from %u to %d", current_move->pos_constraints,
                 cfg_getint(cfg, "pos_constraints"));
        current_move->pos_constraints = cfg_getint(cfg, "pos_constraints");
    }

    if(current_move->next_move != cfg_getint(cfg, "next_move")) {
        log_info("setting next_move from %u to %d", current_move->next_move, cfg_getint(cfg, "next_move"));
        current_move->next_move = cfg_getint(cfg, "next_move");
    }

    if(current_move->successor_id != cfg_getint(cfg, "successor_id")) {
        log_info("setting successor_id from %u to %d", current_move->successor_id, cfg_getint(cfg, "successor_id"));
        current_move->successor_id = cfg_getint(cfg, "successor_id");
    }

    if(current_move->category != cfg_getint(cfg, "category")) {
        log_info("setting category from %u to %d", current_move->category, cfg_getint(cfg, "category"));
        current_move->category = cfg_getint(cfg, "category");
    }

    if(current_move->points != cfg_getint(cfg, "points")) {
        log_info("setting points from %u to %d", current_move->points, cfg_getint(cfg, "points"));
        current_move->points = cfg_getint(cfg, "points");
    }

    if(current_move->block_damage != cfg_getint(cfg, "block_damage")) {
        log_info("setting block_damage from %u to %d", current_move->block_damage, cfg_getint(cfg, "block_damage"));
        current_move->block_damage = cfg_getint(cfg, "block_damage");
    }

    if(current_move->block_stun != cfg_getint(cfg, "block_stun")) {
        log_info("setting block_stun from %u to %d", current_move->block_stun, cfg_getint(cfg, "block_stun"));
        current_move->block_stun = cfg_getint(cfg, "block_stun");
    }

    if(current_move->throw_duration != cfg_getint(cfg, "throw_duration")) {
        log_info("setting throw_duration from %u to %d", current_move->throw_duration,
                 cfg_getint(cfg, "throw_duration"));
        current_move->throw_duration = cfg_getint(cfg, "throw_duration");
    }

    if(current_move->extra_string_selector != cfg_getint(cfg, "extra_string_selector")) {
        log_info("setting extra_string_selector from %u to %d", current_move->extra_string_selector,
                 cfg_getint(cfg, "extra_string_selector"));
        current_move->extra_string_selector = cfg_getint(cfg, "extra_string_selector");
    }

    if(current_move->ani.start_pos.x != cfg_getint(cfg, "start_x")) {
        log_info("setting start_x from %d to %d", current_move->ani.start_pos.x, cfg_getint(cfg, "start_x"));
        current_move->ani.start_pos.x = cfg_getint(cfg, "start_x");
    }

    if(current_move->ani.start_pos.y != cfg_getint(cfg, "start_y")) {
        log_info("setting start_y from %d to %d", current_move->ani.start_pos.y, cfg_getint(cfg, "start_y"));
        current_move->ani.start_pos.y = cfg_getint(cfg, "start_y");
    }

    // Update float fields only if they changed
    if(current_move->damage != cfg_getfloat(cfg, "damage")) {
        log_info("setting damage from %f to %f", current_move->damage, cfg_getfloat(cfg, "damage"));
        current_move->damage = cfg_getfloat(cfg, "damage");
    }

    if(current_move->stun != cfg_getfloat(cfg, "stun")) {
        log_info("setting stun from %f to %f", current_move->stun, cfg_getfloat(cfg, "stun"));
        current_move->stun = cfg_getfloat(cfg, "stun");
    }

    // Update string fields if they were modified
    char *move_str = cfg_getstr(cfg, "move_string");
    if(move_str && strcmp(move_str, str_c(&current_move->move_string)) != 0) {
        log_info("setting move_string from '%s' to '%s'", str_c(&current_move->move_string), move_str);
        str_free(&current_move->move_string);
        str_from_c(&current_move->move_string, move_str);
    }

    char *footer_str = cfg_getstr(cfg, "footer_string");
    if(footer_str && strcmp(footer_str, str_c(&current_move->footer_string)) != 0) {
        log_info("setting footer_string from '%s' to '%s'", str_c(&current_move->footer_string), footer_str);
        str_free(&current_move->footer_string);
        str_from_c(&current_move->footer_string, footer_str);
    }

    char *anim_str = cfg_getstr(cfg, "animation_string");
    if(anim_str && strcmp(anim_str, str_c(&current_move->ani.animation_string)) != 0) {
        log_info("setting animation_string from '%s' to '%s'", str_c(&current_move->ani.animation_string), anim_str);
        str_free(&current_move->ani.animation_string);
        str_from_c(&current_move->ani.animation_string, anim_str);
    }

    cfg_free(cfg);
    return true;
}

bool modmanager_parse_bk_info_mod(const char *buf, bk_info *current_info) {
    if(!buf || !current_info)
        return false;

    cfg_opt_t bk_opts[] = {CFG_INT("chain_hit", current_info->chain_hit, CFGF_NONE),
                           CFG_INT("chain_no_hit", current_info->chain_no_hit, CFGF_NONE),
                           CFG_INT("load_on_start", current_info->load_on_start, CFGF_NONE),
                           CFG_INT("probability", current_info->probability, CFGF_NONE),
                           CFG_INT("hazard_damage", current_info->hazard_damage, CFGF_NONE),
                           CFG_STR("footer_string", str_c(&current_info->footer_string), CFGF_NONE),

                           // Animation-specific options
                           CFG_INT("start_x", current_info->ani.start_pos.x, CFGF_NONE),
                           CFG_INT("start_y", current_info->ani.start_pos.y, CFGF_NONE),
                           CFG_STR("animation_string", str_c(&current_info->ani.animation_string), CFGF_NONE),
                           CFG_END()};

    cfg_t *cfg = cfg_init(bk_opts, CFGF_NONE);

    if(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR) {
        cfg_free(cfg);
        return false;
    }

    // Update integer fields only if they changed
    if(current_info->chain_hit != cfg_getint(cfg, "chain_hit")) {
        log_info("setting chain_hit from %u to %d", current_info->chain_hit, cfg_getint(cfg, "chain_hit"));
        current_info->chain_hit = cfg_getint(cfg, "chain_hit");
    }

    if(current_info->chain_no_hit != cfg_getint(cfg, "chain_no_hit")) {
        log_info("setting chain_no_hit from %u to %d", current_info->chain_no_hit, cfg_getint(cfg, "chain_no_hit"));
        current_info->chain_no_hit = cfg_getint(cfg, "chain_no_hit");
    }

    if(current_info->load_on_start != cfg_getint(cfg, "load_on_start")) {
        log_info("setting load_on_start from %u to %d", current_info->load_on_start, cfg_getint(cfg, "load_on_start"));
        current_info->load_on_start = cfg_getint(cfg, "load_on_start");
    }

    if(current_info->probability != cfg_getint(cfg, "probability")) {
        log_info("setting probability from %u to %d", current_info->probability, cfg_getint(cfg, "probability"));
        current_info->probability = cfg_getint(cfg, "probability");
    }

    if(current_info->hazard_damage != cfg_getint(cfg, "hazard_damage")) {
        log_info("setting hazard_damage from %u to %d", current_info->hazard_damage, cfg_getint(cfg, "hazard_damage"));
        current_info->hazard_damage = cfg_getint(cfg, "hazard_damage");
    }

    if(current_info->ani.start_pos.x != cfg_getint(cfg, "start_x")) {
        log_info("setting start_x from %d to %d", current_info->ani.start_pos.x, cfg_getint(cfg, "start_x"));
        current_info->ani.start_pos.x = cfg_getint(cfg, "start_x");
    }

    if(current_info->ani.start_pos.y != cfg_getint(cfg, "start_y")) {
        log_info("setting start_y from %d to %d", current_info->ani.start_pos.y, cfg_getint(cfg, "start_y"));
        current_info->ani.start_pos.y = cfg_getint(cfg, "start_y");
    }

    // Update string fields if they were modified
    char *footer_str = cfg_getstr(cfg, "footer_string");
    if(footer_str && strcmp(footer_str, str_c(&current_info->footer_string)) != 0) {
        log_info("setting footer_string from '%s' to '%s'", str_c(&current_info->footer_string), footer_str);
        str_free(&current_info->footer_string);
        str_from_c(&current_info->footer_string, footer_str);
    }

    char *anim_str = cfg_getstr(cfg, "animation_string");
    if(anim_str && strcmp(anim_str, str_c(&current_info->ani.animation_string)) != 0) {
        log_info("setting animation_string from '%s' to '%s'", str_c(&current_info->ani.animation_string), anim_str);
        str_free(&current_info->ani.animation_string);
        str_from_c(&current_info->ani.animation_string, anim_str);
    }

    cfg_free(cfg);
    return true;
}

// Helper function to generate mod filename for AF moves
bool modmanager_get_af_move(str *name, int move_id, af_move *move_data) {
    if(!move_data)
        return false;

    str filename;
    str_from_format(&filename, "fighters/%s/%d/animdata.ini", name, move_id);

    list *l;
    unsigned int len;
    bool result = false;
    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
        log_info("HIT %s", str_c(&filename));
        iterator it;
        list_iter_begin(l, &it);
        char *buf;
        foreach(it, buf) {
            result |= modmanager_parse_af_move_mod(buf, move_data);
        }
    }

    str_free(&filename);

    if(!result &&
       (move_id == 7 || move_id == 8 || (move_id >= 12 && move_id <= 14) || (move_id >= 55 && move_id <= 57))) {
        // For fighters, check for 'common' for move_ids 7 (burning oil/stun), 8 (blocking scrape), 12 (scrap), 13
        // (bolt), 14 (screw), 55 (blast), 56 (blast 2), 57 (blast 3)

        str_from_format(&filename, "fighters/common/%d/animdata.ini", move_id);

        if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
            log_info("HIT %s", str_c(&filename));
            iterator it;
            list_iter_begin(l, &it);
            char *buf;
            foreach(it, buf) {
                result |= modmanager_parse_af_move_mod(buf, move_data);
            }
        }
    }

    return result;
}

// Helper function to generate mod filename for BK animations
bool modmanager_get_bk_animation(str *name, int anim_id, bk_info *bk_data) {
    if(!bk_data)
        return false;

    str filename;

    str_from_format(&filename, "scenes/%s/%d/animdata.ini", str_c(name), anim_id);

    list *l;
    unsigned int len = 0;

    bool result = false;
    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
        iterator it;
        list_iter_begin(l, &it);
        char *buf;
        foreach(it, buf) {
            result |= modmanager_parse_bk_info_mod(buf, bk_data);
        }
    }

    str_free(&filename);

    if(!result && strncmp("arena", str_c(name), 5) == 0 &&
       ((anim_id >= 6 && anim_id <= 11) || (anim_id >= 24 && anim_id <= 27))) {
        // TODO make sure this is an arena
        // For arenas, check for 'common' for anim_ids 6 (round), 7 (number), 8 (you lose), 9 (you win), 10 (fight),
        // 11 (ready), 24 (dust 1), 25 (dust 2), 26 (dust 3), 27 (match counters)

        str_from_format(&filename, "scenes/common/%d/animdata.ini", anim_id);
        if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
            iterator it;
            list_iter_begin(l, &it);
            char *buf;
            foreach(it, buf) {
                result |= modmanager_parse_bk_info_mod(buf, bk_data);
            }
        }
    }

    return result;
}

bool modmanager_parse_fighter_header_mod(const char *buf, af *fighter) {

    cfg_opt_t header_opts[] = {CFG_FLOAT("endurance", fighter->endurance, CFGF_NONE),
                               CFG_INT("health", fighter->health, CFGF_NONE),
                               CFG_FLOAT("forward_speed", fighter->forward_speed, CFGF_NONE),
                               CFG_FLOAT("reverse_speed", fighter->reverse_speed, CFGF_NONE),
                               CFG_FLOAT("jump_speed", fighter->jump_speed, CFGF_NONE),
                               CFG_FLOAT("fall_speed", fighter->fall_speed, CFGF_NONE),
                               CFG_END()};

    cfg_t *cfg = cfg_init(header_opts, CFGF_NONE);

    if(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR) {
        cfg_free(cfg);
        return false;
    }

    // Update each field only if it changed, with logging
    if(fighter->endurance != cfg_getfloat(cfg, "endurance")) {
        log_info("setting endurance from %f to %f", fighter->endurance, cfg_getfloat(cfg, "endurance"));
        fighter->endurance = cfg_getfloat(cfg, "endurance");
    }

    if(fighter->health != cfg_getint(cfg, "health")) {
        log_info("setting health from %u to %d", fighter->health, cfg_getint(cfg, "health"));
        fighter->health = cfg_getint(cfg, "health");
    }

    if(fighter->forward_speed != cfg_getfloat(cfg, "forward_speed")) {
        log_info("setting forward speed from %f to %f", fighter->forward_speed, cfg_getfloat(cfg, "forward_speed"));
        fighter->forward_speed = cfg_getfloat(cfg, "forward_speed");
    }

    if(fighter->reverse_speed != cfg_getfloat(cfg, "reverse_speed")) {
        log_info("setting reverse speed from %f to %f", fighter->reverse_speed, cfg_getfloat(cfg, "reverse_speed"));
        fighter->reverse_speed = cfg_getfloat(cfg, "reverse_speed");
    }

    if(fighter->jump_speed != cfg_getfloat(cfg, "jump_speed")) {
        log_info("setting jump speed from %f to %f", fighter->jump_speed, cfg_getfloat(cfg, "jump_speed"));
        fighter->jump_speed = cfg_getfloat(cfg, "jump_speed");
    }

    if(fighter->fall_speed != cfg_getfloat(cfg, "fall_speed")) {
        log_info("setting fall speed from %f to %f", fighter->fall_speed, cfg_getfloat(cfg, "fall_speed"));
        fighter->fall_speed = cfg_getfloat(cfg, "fall_speed");
    }

    cfg_free(cfg);
    return true;
}

// Helper function to load fighter header mod
bool modmanager_get_fighter_header(str *name, af *fighter) {

    str filename;
    str_from_format(&filename, "fighters/%s/header.ini", str_c(name));

    list *l;
    unsigned int len = 0;

    bool result = false;
    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
        iterator it;
        list_iter_begin(l, &it);
        char *buf;
        foreach(it, buf) {
            result |= modmanager_parse_fighter_header_mod(buf, fighter);
        }
    }

    str_free(&filename);
    return result;
}
