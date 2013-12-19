#include <string.h>
#include <shadowdive/shadowdive.h>
#include "resources/af.h"

void af_create(af *a, void *src) {
    sd_af_file *sdaf = (sd_af_file*)src;

    // Trivial stuff
    a->endurance = sdaf->endurance;
    a->power = sdaf->power;
    a->forward_speed = sdaf->forward_speed;
    a->reverse_speed = sdaf->reverse_speed;
    a->jump_speed = sdaf->jump_speed;
    a->fall_speed = sdaf->fall_speed;

    // Sound translation table
    memcpy(a->sound_translation_table, sdaf->soundtable, 30);

    // Moves
    hashmap_create(&a->moves, 8);
    af_move tmp_move;
    for(int i = 0; i < 70; i++) {
        if(sdaf->moves[i] != NULL) {
            af_move_create(&tmp_move, (void*)sdaf->moves[i], i);
            hashmap_iput(&a->moves, i, &tmp_move, sizeof(af_move));
        }
    }
}

af_move* af_get_move(af *a, int id) {
    af_move *val;
    unsigned int tmp;
    if(hashmap_iget(&a->moves, id, (void**)&val, &tmp) == 1) {
        return NULL;
    }
    return val;
}

void af_free(af *a) {
    iterator it;
    hashmap_iter_begin(&a->moves, &it);
    hashmap_pair *pair = NULL;
    while((pair = iter_next(&it)) != NULL) {
        af_move_free((af_move*)pair->val);
    }
    hashmap_free(&a->moves);
}
