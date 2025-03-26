#include "formats/af.h"
#include "resources/af.h"
#include "resources/sprite.h"
#include <string.h>

void af_create(af *a, void *src) {
    sd_af_file *sdaf = (sd_af_file *)src;

    // Trivial stuff
    a->id = sdaf->file_id;
    a->endurance = sdaf->endurance;
    a->health = sdaf->health;
    a->forward_speedf = fixedpt_fromint(sdaf->forward_speed) / 256;
    a->reverse_speedf = fixedpt_fromint(sdaf->reverse_speed) / 256;
    a->jump_speedf = fixedpt_fromint(sdaf->jump_speed) / 256;
    a->fall_speedf = fixedpt_fromint(sdaf->fall_speed) / 256;

    // Sound translation table
    memcpy(a->sound_translation_table, sdaf->soundtable, 30);

    // Set defaults like master.dat
    // TODO: These may change according to pilot and HAR... Find out how.
    a->sound_translation_table[25] = 0;
    a->sound_translation_table[26] = 0;
    a->sound_translation_table[27] = 0;

    array_create(&a->moves);
    array_create(&a->sprites);

    // Moves
    for(int i = 0; i < 70; i++) {
        if(sdaf->moves[i] != NULL) {
            af_move *move = omf_calloc(1, sizeof(af_move));
            af_move_create(move, &a->sprites, (void *)sdaf->moves[i], i);
            array_set(&a->moves, i, move);
        }
    }
}

af_move *af_get_move(const af *a, int id) {
    return array_get(&a->moves, id);
}

void af_free(af *a) {
    iterator it;
    af_move *move = NULL;
    array_iter_begin(&a->moves, &it);
    foreach(it, move) {
        af_move_free(move);
        omf_free(move);
    }
    array_free(&a->moves);
    array_free(&a->sprites);
}
