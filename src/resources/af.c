#include <string.h>
#include <shadowdive/shadowdive.h>
#include "resources/af.h"

void af_create(af *a, void *src) {
    sd_af_file *sdaf = (sd_af_file*)src;

    // Trivial stuff
    a->id = sdaf->file_id;
    a->endurance = sdaf->endurance;
    a->power = sdaf->power;
    a->forward_speed = sdaf->forward_speed;
    a->reverse_speed = sdaf->reverse_speed;
    a->jump_speed = sdaf->jump_speed;
    a->fall_speed = sdaf->fall_speed;

    // Sound translation table
    memcpy(a->sound_translation_table, sdaf->soundtable, 30);

    // Set defaults like master.dat
    a->sound_translation_table[27] = 0;

    // Moves
    for(int i = 0; i < 70; i++) {
        if(sdaf->moves[i] != NULL) {
            af_move_create(&a->moves[i], (void*)sdaf->moves[i], i);
        } else {
            a->moves[i].id = -1;
        }
    }
}

af_move* af_get_move(af *a, int id) {
    if(a->moves[id].id == -1) {
        return NULL;
    }
    return &a->moves[id];
}

void af_free(af *a) {
    for(int i = 0; i < 70; i++) {
        if(a->moves[i].id != -1) {
            af_move_free(&a->moves[i]);
        }
    }
}
