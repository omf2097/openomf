#include <stdlib.h>
#include <string.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/tournament.h"

#define ENEMY_BLOCK_LENGTH 428

sd_tournament_file* sd_tournament_create() {
    sd_tournament_file *trn = malloc(sizeof(sd_tournament_file));
    trn->enemies = NULL;
    trn->enemy_count = 0;
    return trn;
}

int sd_tournament_load(sd_tournament_file *trn, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    trn->enemy_count = sd_read_word(r);
    sd_skip(r, 2);
    trn->victory_text_offset = sd_read_dword(r);
    sd_read_buf(r, trn->bk_name, 14);
    trn->winnings_multiplier = sd_read_float(r);
    sd_skip(r, 4);
    trn->registration_free = sd_read_dword(r);
    trn->assumed_initial_value = sd_read_dword(r);
    trn->tournament_id = sd_read_dword(r);

    // Read enemy block offsets
    sd_reader_set(r, 300);
    int offset_list[trn->enemy_count + 1];
    for(int i = 0; i < trn->enemy_count + 1; i++) {
        offset_list[i] = sd_read_dword(r);
    }

    // Format data
    trn->enemies = malloc(sizeof(sd_tournament_enemy*) * trn->enemy_count);

    // Read enemy data
    char ebuf[ENEMY_BLOCK_LENGTH];
    sd_tournament_enemy *enemy;
    for(int i = 0; i < trn->enemy_count; i++) {
        trn->enemies[i] = malloc(sizeof(sd_tournament_enemy));
        enemy = trn->enemies[i];

        // Find data length
        sd_reader_set(r, offset_list[i]);

        // de-xor data :)
        uint8_t xorkey = ENEMY_BLOCK_LENGTH & 0xFF;
        for(int k = 0; k < ENEMY_BLOCK_LENGTH; k++) {
            ebuf[k] = xorkey++ ^ sd_read_byte(r);
        }

        // Set vars
        enemy->unknown_a = *(uint32_t*)(ebuf + 00);
        memcpy(enemy->name,  ebuf + 4, 18);
        enemy->wins =      *(uint16_t*)(ebuf + 22);
        enemy->losses =    *(uint16_t*)(ebuf + 24);
        enemy->robot_id =  *(uint16_t*)(ebuf + 26);
        memcpy(enemy->stats, ebuf + 28, 8);
        enemy->offense =   *(uint16_t*)(ebuf + 36);
        enemy->defense =   *(uint16_t*)(ebuf + 38);
        enemy->money =     *(uint32_t*)(ebuf + 40);
        enemy->color_1 =   *(uint8_t*) (ebuf + 44);
        enemy->color_2 =   *(uint8_t*) (ebuf + 45);
        enemy->color_3 =   *(uint8_t*) (ebuf + 46);


        // Read quote
        int quote_len = sd_read_word(r);
        enemy->english_quote = malloc(quote_len);
        sd_read_buf(r, enemy->english_quote, quote_len);
    }

/*
    // Print strings at the end
    printf("\nVictory texts:\n");
    sd_reader_set(r, trn->victory_text_offset);
    while(sd_reader_pos(r) < sd_reader_filesize(r)) {
        len = sd_read_word(r);
        if(len == 0) break;
        char buf[len];
        sd_read_buf(r, buf, len);
        printf("[%d] %s\n", len, buf);
    }*/

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_tournament_save(sd_tournament_file *trn, const char *filename) {
    return SD_FILE_OPEN_ERROR;
}

void sd_tournament_delete(sd_tournament_file *trn) {
    for(int i = 0; i < trn->enemy_count; i++) {
        free(trn->enemies[i]->english_quote);
        free(trn->enemies[i]);
    }
    free(trn->enemies);
    free(trn);
}
