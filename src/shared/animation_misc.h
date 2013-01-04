#ifndef _ANIMATION_MISC_H
#define _ANIMATION_MISC_H

#include <shadowdive/shadowdive.h>

int sprite_key_get_id(const char* key);
void sprite_set_key(sd_sprite *s, const char **key, int kcount, const char *value);
void sprite_get_key(sd_sprite *s, const char **key, int kcount);
void sprite_keylist();
void sprite_info(sd_sprite *s, int anim, int sprite);

void anim_common_info(sd_animation *ani);

#endif // _ANIMATION_MISC_H