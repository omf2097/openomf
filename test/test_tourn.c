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

    printf("enemy count   : %d\n", trn->enemy_count);
    printf("v.text. offset: %d\n", trn->victory_text_offset);
    printf("BK name       : %s\n", trn->bk_name);
    printf("win multipl.  : %f\n", trn->winnings_multiplier);
    printf("Reg fee       : %d\n", trn->registration_free);
    printf("initial value : %d\n", trn->assumed_initial_value);
    printf("ID            : %d\n", trn->tournament_id);

    // Print enemy data
    printf("Enemies:");
    for(int i = 0; i < trn->enemy_count; i++) {
        printf("[%d] %s:\n", i, trn->enemies[i]->name);
        printf("  - Wins:     %d\n", trn->enemies[i]->wins);
        printf("  - Losses:   %d\n", trn->enemies[i]->losses);
        printf("  - Robot ID: %d\n", trn->enemies[i]->robot_id);
        printf("  - Offense:  %d\n", trn->enemies[i]->offense);
        printf("  - Defense:  %d\n", trn->enemies[i]->defense);
        printf("  - Money:    %d\n", trn->enemies[i]->money);
        printf("  - Color:    %d,%d,%d\n", 
            trn->enemies[i]->color_1,
            trn->enemies[i]->color_2,
            trn->enemies[i]->color_3);
        printf("  - Stats:    ");
        for(int k = 0; k < 8; k++) {
            printf("%02X ", (uint8_t)trn->enemies[i]->stats[k]);
        }
        printf("\n");
        printf("  - Quote:    %s\n", trn->enemies[i]->english_quote);
    }
    
    sd_tournament_delete(trn);
    return 0;
}
