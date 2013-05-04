#include "video/texturelist.h"
#include "utils/list.h"

list texturelist;

void texturelist_init() {
    list_create(&texturelist);
}

void texturelist_close() {
    list_free(&texturelist);
}

void texturelist_add(texture *tex) {
    list_append(&texturelist, &tex, sizeof(texture*));
}

void texturelist_remove(texture *tex) {
    iterator it;
    list_iter_begin(&texturelist, &it);
    texture **tmp = NULL;
    while((tmp = iter_next(&it)) != NULL) {
        if(*tmp == tex) {
            list_delete(&texturelist, &it);
            return;
        }
    }
}

void texturelist_revalidate_all() {
    iterator it;
    list_iter_begin(&texturelist, &it);
    texture **tex = NULL;
    while((tex = iter_next(&it)) != NULL) {
        texture_revalidate(*tex);
    }
}

unsigned int texturelist_get_bsize() {
    unsigned int bytes = 0;
    iterator it;
    list_iter_begin(&texturelist, &it);
    texture **tex = NULL;
    while((tex = iter_next(&it)) != NULL) {
        bytes += texture_size(*tex);
    }
    return bytes;
}