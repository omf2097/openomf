#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helpers.h"

const char* mstr[] = {
    "PUNCH",
    "KICK",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT"
};

void print_key(char *o, uint8_t key) {
    int pos = 0;
    o[0] = 0;
    for(int i = 0; i < 6; i++) {
        uint8_t m = 1 << i;
        if(key & m) {
            if(pos > 0) {
                pos += sprintf((char*)(o+pos), "|");
            }
            pos += sprintf((char*)(o+pos), "%s", mstr[i]);
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
        print_pilot_info(&rec->pilots[i]);
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

    printf("## Parsed data:\n");
    printf("Number   Tick Extra Player Action        Action enum  Extra data\n");
    for(int i = 0; i < rec->move_count; i++) {
        char tmp[100];
        tmp[0] = 0;
        if(rec->moves[i].extra < 3) {
            print_key(tmp, rec->moves[i].action);
        }
        printf(" - %3d: %5d %5d %6d %6d %18s",
            i,
            rec->moves[i].tick,
            rec->moves[i].extra,
            rec->moves[i].player_id,
            rec->moves[i].raw_action,
            tmp);

        if(rec->moves[i].extra > 2) {
            print_bytes(rec->moves[i].extra_data, 7, 8, 2);
        }
        printf("\n");
    }

    sd_rec_free(rec);
    free(rec);
    return 0;
}
