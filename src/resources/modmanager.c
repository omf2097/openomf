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

hashmap mod_resources;

typedef struct {
    size_t size;
    unsigned char *buf;
} opus_buffer;

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

bool modmanager_init(void) {

    hashmap_create(&mod_resources);
    list dir_list;
    list_create(&dir_list);
    mod_find(&dir_list);

    iterator it;
    list_iter_begin(&dir_list, &it);
    path *p;
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
                        sd_vga_image img;
                        if(sd_vga_image_from_png_in_memory(&img, entry_buf, entry_size, false) == SD_SUCCESS) {

                            log_info("got vga image %dx%d with size %d", img.w, img.h, sizeof(img));

                            hashmap_put_str(&mod_resources, str_c(&filename), &img, sizeof(img));
                        } else {
                            log_warn("failed to load background image %s", str_c(&filename));
                        }
                    } else if(strcmp(".png", str_c(&ext)) == 0) {
                        // parse as sprite
                        sd_vga_image img;
                        if(sd_vga_image_from_png_in_memory(&img, entry_buf, entry_size, true) == SD_SUCCESS) {
                            sd_sprite s;
                            if(strcmp("7_0.png", str_c(&fn)) == 0) {
                                log_warn("pixel 0 has pallete %d", img.data[0]);
                            }
                            if(sd_sprite_vga_encode(&s, &img) == SD_SUCCESS) {
                                hashmap_put_str(&mod_resources, str_c(&filename), &s, sizeof(s));
                            } else {
                                log_warn("failed to load sprite %s", str_c(&filename));
                            }
                        }
#ifdef OPUSFILE_FOUND
                    } else if(strcmp(".ogg", str_c(&ext)) == 0) {
                        if(op_test(NULL, entry_buf, entry_size) == 0) {
                            log_info("got OPUS file %s", str_c(&filename));
                            // TODO definitely make this a list
                            list *l;
                            unsigned int len;
                            opus_buffer *buf = omf_calloc(1, sizeof(opus_buffer));
                            buf->size = entry_size;
                            buf->buf = entry_buf;
                            if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
                                list_append(l, buf, sizeof(opus_buffer));
                            } else {
                                l = omf_calloc(1, sizeof(list));
                                list_create(l);
                                list_append(l, buf, sizeof(opus_buffer));
                                hashmap_put_str(&mod_resources, str_c(&filename), l, sizeof(list));
                            }
                        } else {
                            log_warn("Failed to parse OPUS file %s", str_c(&filename));
                        }
#endif
                    }

                    // TODO each filename should be a list
                    // TODO each filetype should be parsed (eg png to sprite, etc)
                }
            }
        } else {
            log_warn("mod %s has no contents", path_c(p));
        }
    }

    list_free(&dir_list);
    return true;
}

bool modmanager_get_bk_background(int file_id, sd_vga_image **img) {
    str filename;
    switch(file_id) {
        case 8:
            str_from_c(&filename, "arenas/arena0/background.png");
            break;
        case 16:
            str_from_c(&filename, "arenas/arena1/background.png");
            break;
        case 32:
            str_from_c(&filename, "arenas/arena2/background.png");
            break;
        case 64:
            str_from_c(&filename, "arenas/arena3/background.png");
            break;
        case 128:
            str_from_c(&filename, "arenas/arena4/background.png");
            break;
        default:
            return false;
    }

    unsigned int len;
    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)img, &len)) {
        log_info("got vga image %dx%d with size %d", (*img)->w, (*img)->h, len);
        return true;
    }
    return false;
}

bool modmanager_get_sprite(animation_source source, int file_id, int animation, int frame, sd_sprite **spr) {
    str filename;
    // TODO for arenas, check for 'common' for animations 6 (round), 7 (number), 8 (you lose), 9 (you win), 10 (fight),
    // 11 (ready), 24 (dust 1), 25 (dust 2), 26 (dust 3), 27 (match counters)
    // TODO for fighters, check for 'common' for animations 12 (scrap), 13 (bolt), 14 (screw), 55 (blast), 56 (blast 2),
    // 57 (blast 3)
    //
    // Common replacements should replace default assets, but not modded ones
    switch(source) {
        case AF_ANIMATION:
            str_from_format(&filename, "fighters/fighter%d/%d/%d_%d.png", file_id, animation, animation, frame);
            break;
        case BK_ANIMATION: {
            switch(file_id) {
                case 8:
                    str_from_format(&filename, "arenas/arena0/%d/%d_%d.png", animation, animation, frame);
                    break;
                case 16:
                    str_from_format(&filename, "arenas/arena1/%d/%d_%d.png", animation, animation, frame);
                    break;
                case 32:
                    str_from_format(&filename, "arenas/arena2/%d/%d_%d.png", animation, animation, frame);
                    break;
                case 64:
                    str_from_format(&filename, "arenas/arena3/%d/%d_%d.png", animation, animation, frame);
                    break;
                case 128:
                    str_from_format(&filename, "arenas/arena4/%d/%d_%d.png", animation, animation, frame);
                    break;
                default:
                    return false;
            }
            break;
        }
        default:
            return false;
    }

    unsigned int len;
    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)spr, &len)) {
        log_info("got sprite %dx%d with size %d", (*spr)->width, (*spr)->height, len);
        return true;
    }

    // log_warn("MISS for sprite %s", str_c(&filename));

    return false;
}

unsigned int modmanager_count_music(str *name) {
    str filename;

    str_from_format(&filename, "audio/common/music/%s.ogg", str_c(name));
    str_tolower(&filename);

    list *l;
    unsigned int len = 0;

    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
        return list_size(l);
    }

    return 0;
}

bool modmanager_get_music(str *name, unsigned int index, unsigned char **buf, size_t *buflen) {
    str filename;

    str_from_format(&filename, "audio/common/music/%s.ogg", str_c(name));
    str_tolower(&filename);

    list *l;
    unsigned int len = 0;

    if(!hashmap_get_str(&mod_resources, str_c(&filename), (void **)&l, &len)) {
        unsigned int count = list_size(l);
        log_info("found %d music files for %s", count, name);
        if(index >= count) {
            log_warn("requested index %s into list of %d members", index, count);
            return false;
        }
        opus_buffer *obuf = list_get(l, index);
        assert(obuf != NULL);
        *buf = obuf->buf;
        *buflen = obuf->size;
        return true;
    }

    log_warn("MISS for %s", str_c(&filename));

    return false;
}
