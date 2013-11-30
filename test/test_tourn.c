#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("test_tourn <tournamentfile>\n");
        return 0;
    }
    
    sd_tournament_file *trn = sd_tournament_create();
    if(sd_tournament_load(trn, argv[1])) {
        printf("Tournament file could not be loaded!\n");
        return 1;
    }

    printf("enemy count: %d\n", trn->enemy_count);

    printf("a          : %d\n", trn->unknown_a);
    printf("v.text.off.: %d\n", trn->victory_text_offset);

    printf("bk_name    : %s\n", trn->bk_name);

    printf("win multip.: %f\n", trn->winnings_multiplier);
    printf("b          : %d\n", trn->unknown_b);

    printf("reg fee    : %d\n", trn->registration_free);
    printf("init. val  : %d\n", trn->assumed_initial_value);
    printf("ID         : %d\n", trn->tournament_id);

    // Print offsets (temporary)
    printf("Offset section:\n");
    for(int i = 0; i < trn->enemy_count + 1; i++) {
        printf(" - Offset %d: %d\n", i, trn->offset_list[i]);
    }
    
    sd_tournament_delete(trn);
    return 0;
}
