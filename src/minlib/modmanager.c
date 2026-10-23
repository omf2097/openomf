/* MIN_LIB_BUILD does not load mods. Callers keep the original game data. */
#ifndef MIN_LIB_BUILD
#error "modmanager stand-in is part of MIN_LIB_BUILD"
#endif

#include "resources/modmanager.h"
#include "utils/log.h"

bool modmanager_init(void) {
    log_info("Mods are disabled.");
    return true;
}

void modmanager_shutdown(void) {
}

void modmanager_set_allowed(bool allowed) {
    (void)allowed;
}

bool modmanager_get_bk_background(str *name, sd_vga_image **img) {
    (void)name;
    (void)img;
    return false;
}

bool modmanager_get_sprite(animation_source source, str *name, int animation, int frame, sd_sprite **spr) {
    (void)source;
    (void)name;
    (void)animation;
    (void)frame;
    (void)spr;
    return false;
}

unsigned int modmanager_count_music(str *name) {
    (void)name;
    return 0;
}

bool modmanager_get_music(str *name, unsigned int index, unsigned char **buf, size_t *buflen) {
    (void)name;
    (void)index;
    (void)buf;
    (void)buflen;
    return false;
}

bool modmanager_get_hitcoords(animation_source source, str *name, int animation, int frame, vector *coords,
                              vec2i *origin, vec2i *sprite_offset) {
    (void)source;
    (void)name;
    (void)animation;
    (void)frame;
    (void)coords;
    (void)origin;
    (void)sprite_offset;
    return false;
}

bool modmanager_get_af_move(str *name, int move_id, af_move *move_data) {
    (void)name;
    (void)move_id;
    (void)move_data;
    return false;
}

bool modmanager_get_bk_animation(str *name, int anim_id, bk_info *bk_data) {
    (void)name;
    (void)anim_id;
    (void)bk_data;
    return false;
}

bool modmanager_get_fighter_header(str *name, af *fighter) {
    (void)name;
    (void)fighter;
    return false;
}

bool modmanager_get_tournament_mod(const char *tournament_name, sd_tournament_file *tourn_data) {
    (void)tournament_name;
    (void)tourn_data;
    return false;
}

bool modmanager_parse_photo_mod(const char *buf, sd_pic_photo *photo) {
    (void)buf;
    (void)photo;
    return false;
}

bool modmanager_get_player_pics(sd_pic_file *pic) {
    (void)pic;
    return false;
}
