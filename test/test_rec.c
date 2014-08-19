#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_bytes(char *buf, int len, int line, int padding) {
    for(int k = 0; k < padding; k++) {
        printf(" ");
    }
    for(int i = 1; i <= len; i++) {
        printf("%02x ", (uint8_t)buf[i-1]);
        if(i % line == 0) {
            printf("\n");
            for(int k = 0; k < padding; k++) {
                printf(" ");
            }
        }
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("test_rec <recfile>\n");
        return 0;
    }
    
    // Open tournament file
    sd_rec_file *rec = malloc(sizeof(sd_rec_file));
    sd_rec_create(rec);
    int ret = sd_rec_load(rec, argv[1]);
    if(ret != SD_SUCCESS) {
        printf("Shadowdive error %d: %s\n", ret, sd_get_error(ret));
        return 1;
    }

    // Print enemy data
    printf("Enemies:\n");
    for(int i = 0; i < 2; i++) {
        sd_pilot *pilot = rec->pilots[i];

        printf("### Pilot header for [%d] %s:\n", i, pilot->name);
        printf("  - Wins:        %d\n", pilot->wins);
        printf("  - Losses:      %d\n", pilot->losses);
        printf("  - Robot ID:    %d\n", pilot->robot_id);
        printf("  - Offense:     %d\n", pilot->offense);
        printf("  - Defense:     %d\n", pilot->defense);
        printf("  - Money:       %d\n", pilot->money);
        printf("  - Color:       %d,%d,%d\n", 
            pilot->color_1,
            pilot->color_2,
            pilot->color_3);

        printf("  - Stats:       ");
        print_bytes(pilot->stats, 8, 10, 0);
        printf("\n");
        printf("  - unk_block_a:\n");
        print_bytes(pilot->unk_block_a, 107, 16, 5);
        printf("\n");
        printf("  - Force arena: %d\n", pilot->force_arena);
        printf("  - unk_block_b: ");
        print_bytes(pilot->unk_block_a, 3, 16, 0);
        printf("\n");
        printf("  - Movement:    %d\n", pilot->movement);
        printf("  - unk_block_c: ");
        print_bytes(pilot->unk_block_c, 6, 16, 0);
        printf("\n");
        printf("  - Enhancement: ");
        print_bytes(pilot->enhancements, 11, 16, 0);
        printf("\n");
        printf("  - Flags:       %d\n", pilot->flags);

        printf("  - Reqs:        ");
        print_bytes((char*)pilot->reqs, 10, 16, 0);
        printf("\n");
        printf("  - Attitude:    ");
        print_bytes((char*)pilot->attitude, 6, 16, 0);
        printf("\n");
        printf("  - unk_block_d: ");
        print_bytes(pilot->unk_block_d, 6, 16, 0);
        printf("\n");

        printf("  - AP Throw:    %d\n", pilot->ap_throw);
        printf("  - AP Special:  %d\n", pilot->ap_special);
        printf("  - AP Jump:     %d\n", pilot->ap_jump);
        printf("  - AP High:     %d\n", pilot->ap_high);
        printf("  - AP Low:      %d\n", pilot->ap_low);
        printf("  - AP Middle:   %d\n", pilot->ap_middle);

        printf("  - Pref jump    %d\n", pilot->pref_jump);
        printf("  - Pref fwd     %d\n", pilot->pref_fwd);
        printf("  - Pref back    %d\n", pilot->pref_back);

        printf("  - unk_block_e: ");
        print_bytes(pilot->unk_block_d, 4, 16, 0);
        printf("\n");

        printf("  - Learning     %f\n", pilot->learning);
        printf("  - Forget       %f\n", pilot->forget);
        printf("  - Winnings     %d\n", pilot->winnings);
        printf("  - Photo ID     %d\n", pilot->photo_id);

        printf("  - unk_block_f: ");
        print_bytes(pilot->unk_block_f, 24, 26, 0);
        printf("\n");
        printf("  - unk_block_g:\n");
        print_bytes(pilot->unk_block_g, 166, 16, 5);
        printf("\n\n");
    }

    char tmp = 'A';
    printf("## Unknown header data:\n");
    printf("  - Score A: %d\n", rec->scores[0]);
    printf("  - Score B: %d\n", rec->scores[1]);
    printf("  - %c:       %d\n", tmp++, rec->unknown_a);
    printf("  - %c:       %d\n", tmp++, rec->unknown_b);
    printf("  - %c:       %d\n", tmp++, rec->unknown_c);
    printf("  - %c:       %d\n", tmp++, rec->unknown_d);
    printf("  - %c:       %d\n", tmp++, rec->unknown_e);
    printf("  - %c:       %d\n", tmp++, rec->unknown_f);
    printf("  - %c:       %d\n", tmp++, rec->unknown_g);
    printf("  - %c:       %d\n", tmp++, rec->unknown_h);
    printf("  - %c:       %d\n", tmp++, rec->unknown_i);
    printf("  - %c:       %d\n", tmp++, rec->unknown_j);
    printf("  - %c:       %d\n", tmp++, rec->unknown_k);
    printf("  - %c:       %d\n", tmp++, rec->unknown_l);
    printf("  - %c:       %d\n", tmp++, rec->unknown_m);
    printf("\n");

    printf("## Move data:\n");
    print_bytes((char*)rec->raw, rec->rawsize, 7, 3);
    printf("\n");

    printf("## Parsed data:\n");
    printf("Number    Tick Extra Player Action   Extra data\n");
    for(int i = 0; i < rec->move_count; i++) {
        printf("  - %3d: %5d %5d %6d %6d",
            i,
            rec->moves[i].tick,
            rec->moves[i].extra,
            rec->moves[i].player_id,
            rec->moves[i].action);
        if(rec->moves[i].extra > 2) {
            print_bytes(rec->moves[i].extra_data, 7, 8, 3);
        }
        printf("\n");
    }
    printf("\n");

    sd_rec_free(rec);
    free(rec);
    return 0;
}
